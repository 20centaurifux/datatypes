#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "datatypes.h"

int
main(int argc, char *argv[])
{
	Buffer *buf;
	int fd;
	uint32_t i;
	ssize_t result;
	bool success;
	size_t len = 0;
	char *dst = NULL;

	// initialize tree:
	buf = buffer_new(4096);

	assert(buf != NULL);
	assert(buffer_is_valid(buf));
	assert(buffer_is_empty(buf));

	if((fd = open("bible.txt", O_RDONLY)) >= 0)
	{
		for(i = 0; i < 10; ++i)
		{
			result = buffer_fill_from_fd(buf, fd, 256);

			assert(result == 256);
			assert(buffer_is_valid(buf));
			assert(!buffer_is_empty(buf));
			assert(buffer_len(buf) == (i + 1) * 256);
		}

		buffer_clear(buf);

		assert(buffer_is_valid(buf));
		assert(buffer_is_empty(buf));

		result = buffer_fill_from_fd(buf, fd, 4096);

		assert(result == 4096);
		assert(buffer_len(buf) == 4096);
		assert(buffer_is_valid(buf));
		assert(!buffer_is_empty(buf));

		success = buffer_fill(buf, "foobar", 6);

		assert(!success);
		assert(!buffer_is_valid(buf));
		assert(buffer_len(buf) == 0);

		buffer_clear(buf);

		assert(buffer_is_valid(buf));
		assert(buffer_is_empty(buf));
		assert(buffer_len(buf) == 0);

		success = buffer_fill(buf, "foo\nbar", 7);

		assert(success == true);

		success = buffer_read_line(buf, &dst, &len);

		assert(success == true);
		assert(!strcmp(dst, "foo"));
		assert(len == 3);

		success = buffer_read_line(buf, &dst, &len);

		assert(success == false);
		assert(!strcmp(dst, "foo"));
		assert(len == 3);

		success = buffer_flush(buf, &dst, &len);

		assert(success == true);
		assert(!strcmp(dst, "bar"));
		assert(len == 3);

		free(dst);

		close(fd);
	}
	else
	{
		fprintf(stderr, "Couldn't read bible.txt.\n");
	}

	buffer_destroy(buf);
}

