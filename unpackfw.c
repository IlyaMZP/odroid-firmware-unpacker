#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define FIRMWARE_DESCRIPTION_SIZE (40)
#define TILE_LENGTH (8256)
char FirmwareDescription[FIRMWARE_DESCRIPTION_SIZE];
size_t count;
const char* HEADER_V00_01 = "ODROIDGO_FIRMWARE_V00_01";

typedef struct
{
	uint8_t type;
	uint8_t subtype;
	uint8_t _reserved0;
	uint8_t _reserved1;

	uint8_t label[16];

	uint32_t flags;
	uint32_t length;
} odroid_partition_t;


int main(int argc, char *argv[])
{

	FILE* file = fopen(argv[1], "rb");

	// Check the header
	const size_t headerLength = strlen(HEADER_V00_01);
	char* header = malloc(headerLength + 1);
	if(!header)
	{
		printf("MEMORY ERROR\n");
		abort();
	}

	// null terminate
	memset(header, 0, headerLength + 1);

	count = fread(header, 1, headerLength, file);
	if (count != headerLength)
	{
		printf("HEADER READ ERROR\n");
		abort();
	}

	if (strncmp(HEADER_V00_01, header, headerLength) != 0)
	{
		printf("HEADER MATCH ERROR\n");
		abort();
	}

	printf("Header OK: '%s'\n", header);
	free(header);

	// read description
	count = fread(FirmwareDescription, 1, FIRMWARE_DESCRIPTION_SIZE, file);
	if (count != FIRMWARE_DESCRIPTION_SIZE)
	{
		printf("DESCRIPTION READ ERROR");
		abort();
	}

	// ensure null terminated
	FirmwareDescription[FIRMWARE_DESCRIPTION_SIZE - 1] = 0;

	printf("FirmwareDescription='%s'\n", FirmwareDescription);

	// Tile
	uint16_t* tileData = malloc(TILE_LENGTH);
	if (!tileData)
	{
		printf("TILE MEMORY ERROR");
		abort();
	}

	count = fread(tileData, 1, TILE_LENGTH, file);
	if (count != TILE_LENGTH)
	{
		printf("TILE READ ERROR");
		abort();
	}

	FILE* tile = fopen("tile.raw" , "wb");
	if (!tile) abort();

	fwrite(tileData, TILE_LENGTH, 1, tile);

	fclose(tile);
	free(tileData);

	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (fseek(file, 8320, SEEK_SET) != 0)
	{
		printf("SEEK ERROR\n\n");
		abort();
	}

	const int ERASE_BLOCK_SIZE = 4096;
	void* data = malloc(ERASE_BLOCK_SIZE);
	if (!data)
	{
		printf("DATA MEMORY ERROR\n");
		abort();
	}

	while(1)
	{
		if (ftell(file) >= (fileSize - sizeof(uint32_t)))
		{
			break;
		}

		// Read Partition Info
		odroid_partition_t slot;
		count = fread(&slot, 1, sizeof(slot), file);
		if (count != sizeof(slot))
		{
			printf("PARTITION READ ERROR\n");
			abort();
		}

		printf("type=%x, subtype=%x, length=%d, label=%-16s\n",
			slot.type, slot.subtype, slot.length, slot.label);

		// Read Data Length
		uint32_t length;
		count = fread(&length, 1, sizeof(length), file);
		if (count != sizeof(length))
		{
			printf("LENGTH READ ERROR\n");
			abort();
		}


		size_t nextEntry = ftell(file) + length;

		if (length > 0)
		{
			// Create Output File
			FILE* output = fopen(slot.label , "wb");
			if (!output) abort();

			for (int offset = 0; offset < length; offset += ERASE_BLOCK_SIZE)
			{
				// Read Input File to Buffer
				count = fread(data, 1, ERASE_BLOCK_SIZE, file);
				if (count <= 0)
				{
					printf("DATA READ ERROR\n");
					abort();
				}
				fwrite(data, ERASE_BLOCK_SIZE, 1, output);
			}
			fclose(output);
		}

		// Seek to next entry
		if (fseek(file, nextEntry, SEEK_SET) != 0)
		{
			printf("SEEK ERROR\n\n");
			abort();
		}
	}
	free(data);
	fclose(file);
	return 0;
}
