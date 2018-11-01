#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHUNKID		"RIFF"
#define FORMAT		"WAVE"
#define SUBCHUNK1ID	"fmt "
#define SUBCHUNK1SIZE	16
#define PCM		1
#define MONO		1
#define SAMPLE_RATE	44100
#define BITS_PER_SAMPLE	16
#define BITS_PER_BYTE	8
#define BITS_PER_WORD	32
#define BLOCK_ALIGN	2
#define SUBCHUNK2ID	"data"

#define HEADER_SIZE	44

#define CHKID_OFFSET	0
#define CHKSZ_OFFSET	4
#define FORMT_OFFSET	8
#define SC1ID_OFFSET	12
#define SC1SZ_OFFSET	16
#define ADFMT_OFFSET	20
#define NMCHN_OFFSET	22
#define SAMPRT_OFFSET	24
#define BYTERT_OFFSET	28
#define BLKALN_OFFSET	32
#define	BPSAMP_OFFSET	34
#define SC2ID_OFFSET	36
#define SC2SZ_OFFSET	40

#define CHKID_SIZE	4
#define FORMT_SIZE	4
#define SC1ID_SIZE	4
#define SC2ID_SIZE	4

#define LSHIFT		8

#define FAILURE		-1
#define SUCCESS		0

struct wav_header {
	char chunk_id[CHKID_SIZE+1];
	uint32_t chunk_size;
	char format[FORMT_SIZE+1];
	char sc1_id[SC1ID_SIZE+1];
	uint32_t sc1_size;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bpsample;
	char sc2_id[SC2ID_SIZE+1];
	uint32_t sc2_size;
};

static bool parse_header(int fd, struct wav_header * hdr);
static bool check_header(struct wav_header * hdr);

int driver;	// file descriptor for device driver

int sound_init(const char * device) {
	// open character device specified
	if ((driver = open(device, O_RDWR)) < 0) {
		fprintf(stderr, "Could not open \"%s\": %s\n", device, strerror(errno));
		return FAILURE;
	}
	return SUCCESS;
}

int sound_play(const char * wav_file)
{
	int fd, status = FAILURE;
	size_t nread;			// number of bytes read at once
	uint32_t index = 0;		// used for scratch work
	uint32_t audio_index = 0;	// used to decode audio data
	uint32_t tmp;			// used to store each sample
	struct wav_header header;	// holds the metadata from the header
	uint8_t * data;			// holds raw audio data
	uint32_t * audio;		// holds parsed audio data

	// open the WAV file
	if ((fd = open(wav_file, O_RDONLY)) < 0) {
		fprintf(stderr, "Could not open %s: %s\n", wav_file, strerror(errno));
		return fd;
	}

	// parse header and make sure it's valid
	if (!parse_header(fd, &header) || !check_header(&header)) {
		goto bad_hdr;
	}

	//printf("ChunkID: \"%s\"\n", header.chunk_id);
	//printf("ChunkSize: %d\n", header.chunk_size);
	//printf("Format: \"%s\"\n", header.format);
	//printf("Subchunk1ID: \"%s\"\n", header.sc1_id);
	//printf("Subchunk1Size: %d\n", header.sc1_size);
	//printf("AudioFormat: %d\n", header.audio_format);
	//printf("NumChannels: %d\n", header.num_channels);
	//printf("SampleRate: %d\n", header.sample_rate);
	//printf("ByteRate: %d\n", header.byte_rate);
	//printf("BlockAlign: %d\n", header.block_align);
	//printf("BitsPerSample: %d\n", header.bpsample);
	//printf("Subchunk2ID: \"%s\"\n", header.sc2_id);
	//printf("Subchunk2Size: %d\n", header.sc2_size);

	// open audio file
	if ( (data = malloc(header.sc2_size)) == NULL ) {
		fprintf(stderr, "Call to malloc() failed: %s\n", strerror(errno));
		goto err_malloc;
	}

	// read data in from file
	while (index < header.sc2_size) {
		// check number of bytes read
		nread = read(fd, &data[index], header.sc2_size-index);

		if (nread < 0) {
			// an error occurred
			fputs(strerror(errno), stderr);
			goto err_read;
		} else if (nread == 0) {
			// reached end of file
			fprintf(stderr, "Could not read full .wav file: %d bytes read\n", index);
			goto err_read;
		} else {
			// successfully got nread bytes
			index += nread;
		}
	}

	// allocate 4 bytes for every sample
	if ( (audio = malloc(header.sc2_size * BITS_PER_WORD / BITS_PER_SAMPLE)) == NULL) {
		goto err_audio;
	}

	// for each sample
	for (index = 0; index < header.sc2_size; index += BITS_PER_SAMPLE/BITS_PER_BYTE) {
		// decode, with implicit cast to uint32_t
		tmp = le16toh( *((uint16_t *) &data[index]) );

		// leftshift to match expected format
		audio[audio_index++] = tmp << LSHIFT;
	}

	// write the full audio data to the driver
	if (write(driver, audio, header.sc2_size * BITS_PER_WORD / BITS_PER_SAMPLE) < 0) {
		goto err_write;
	}

	// any error would skip past this with goto
	status = SUCCESS;

// undo operations in reverse order if something fails
err_write:
	free(audio);

err_audio:
err_read:
	free(data);

err_malloc:
bad_hdr:
	close(fd);

	return status;
}

static bool parse_header(int fd, struct wav_header * hdr)
{
	unsigned char buf[HEADER_SIZE];		// holds raw buffer data
	ssize_t nread;				// counts bytes read
	size_t index = 0;			// used for scratch work

	// read a full HEADER_SIZE bytes
	while (index < HEADER_SIZE) {
		// read at the current index
		nread = read(fd, &buf[index], HEADER_SIZE-index);
		if (nread < 0) {
			// some error occurred
			fputs(strerror(errno), stderr);
			return false;
		} else if (nread == 0) {
			// reached end of file
			fprintf(stderr, "Header too short: %d bytes read\n", index);
			return false;
		} else {
			// read operation was successful
			index += nread;
		}
	}
	
	// this assumes everything is at the expected address
	// call check_header to make sure it's valid
	strncpy(hdr->chunk_id, &buf[CHKID_OFFSET], CHKID_SIZE);
	hdr->chunk_size = le32toh( *((uint32_t *) &buf[CHKSZ_OFFSET]) );
	strncpy(hdr->format, &buf[FORMT_OFFSET], FORMT_SIZE);
	strncpy(hdr->sc1_id, &buf[SC1ID_OFFSET], SC1ID_SIZE);
	hdr->sc1_size = le32toh( *((uint32_t *) &buf[SC1SZ_OFFSET]) );
	hdr->audio_format = le16toh( *((uint16_t *) &buf[ADFMT_OFFSET]));
	hdr->num_channels = le16toh( *((uint16_t *) &buf[NMCHN_OFFSET]));
	hdr->sample_rate = le32toh( *((uint32_t *) &buf[SAMPRT_OFFSET]) );
	hdr->byte_rate = le32toh( *((uint32_t *) &buf[BYTERT_OFFSET]) );
	hdr->block_align = le16toh( *((uint16_t *) &buf[BLKALN_OFFSET]) );
	hdr->bpsample = le16toh( *((uint16_t *) &buf[BPSAMP_OFFSET]) );
	strncpy(hdr->sc2_id, &buf[SC2ID_OFFSET], SC2ID_SIZE);
	hdr->sc2_size = le32toh( *((uint32_t *) &buf[SC2SZ_OFFSET]) );

	// add null characters to the end of all the strings
	hdr->chunk_id[CHKID_SIZE] = '\0';
	hdr->format[FORMT_SIZE] = '\0';
	hdr->sc1_id[SC1ID_SIZE] = '\0';
	hdr->sc2_id[SC2ID_SIZE] = '\0';

	// parse successful
	return true;
}

static bool check_header(struct wav_header * hdr) {
	if (strcmp(hdr->chunk_id, CHUNKID)) {
		fputs("Unknown ChunkID", stderr);
	} else if (strcmp(hdr->format, FORMAT)) {
		fputs("Unknown Format", stderr);
	} else if (strcmp(hdr->sc1_id, SUBCHUNK1ID)) {
		fputs("Unknown Subchunk1ID", stderr);
	} else if (hdr->sc1_size != SUBCHUNK1SIZE) {
		fprintf(stderr, "Unexpected Subchunk1Size: %d\n", hdr->sc1_size);
	} else if (hdr->audio_format != PCM) {
		fprintf(stderr, "Unsupported audio format: %d\n", hdr->audio_format);
	} else if (hdr->num_channels != MONO) {
		fprintf(stderr, "Unsupported number of channels: %d\n", hdr->num_channels);
	} else if (hdr->sample_rate != SAMPLE_RATE) {
		fprintf(stderr, "Unsupported sample rate: %d\n", hdr->sample_rate);
	} else if (hdr->bpsample != BITS_PER_SAMPLE) {
		fprintf(stderr, "Unexpected bits per sample: %d\n", hdr->bpsample);
	} else if (hdr->byte_rate * BITS_PER_BYTE != hdr->sample_rate * hdr->num_channels * hdr->bpsample) {
		fputs("Byte Rate does not match expected value", stderr);
	} else if (hdr->block_align != BLOCK_ALIGN) {
		fputs("Bad block align", stderr);
	} else if (strcmp(hdr->sc2_id, SUBCHUNK2ID)) {
		fputs("Unknown Subchunk2ID", stderr);
	} else {
		return true;
	}
	return false;
}

void sound_wait() {
	// busy wait as long as read returns 1
	while (read(driver, NULL, 0));
}

void sound_exit() {
	// close character device file
	close(driver);
}
