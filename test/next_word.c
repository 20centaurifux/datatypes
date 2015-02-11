#include <stdio.h>
#include <string.h>
#include <ctype.h>

const char *
next_word(FILE *fp)
{
	static char word[32];
	size_t len;

	// read next word from file:
	if(fscanf(fp, "%s", word) != 1)
	{
		return NULL;
	}

	// remove non-alphanumeric characters from start & end of string:
	len = strlen(word);

	while(len > 0 && !isalnum(word[0]))
	{
		memmove(word, word + 1, --len);
	}

	while(len > 0 && !isalnum(word[len - 1]))
	{
		word[len - 1] = '\0';
		--len;
	}

	// test if found word is empty...
	if(len)
	{
		// word is not empty => convert characters to lower case and return word:
		for(size_t i = 0; i < len; ++i)
		{
			word[i] = tolower(word[i]);
		}

		return word;
	}

	// found word is empty => find next one:
	return next_word(fp);
}

