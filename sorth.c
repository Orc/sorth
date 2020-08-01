/*
 * sort the output from du -sh
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

char **lines = NULL;
int nrlines = 0;
int szlines = 80;

int reversed = 0;


/*
 * allocate memory or die
 */
void *
xmalloc(size_t size)
{
    void *res = malloc(size);

    if ( res ) return res;

    fprintf(stderr, "unable to allocate %ld byte%s (%s)\n", (long)size, (size==1)?"":"s", strerror(errno));
    exit(1);
}


/*
 * reallocate memory or die
 */
void *
xrealloc(void *ptr, size_t newsize)
{
    void *res = realloc(ptr, newsize);

    if ( res ) return res;

    fprintf(stderr, "unable to reallocate %ld byte%s (%s)\n", (long)newsize, (newsize==1)?"":"s", strerror(errno));
    exit(1);
}


/*
 * copy size bytes of a string into malloced memory, or die
 */
char *
xstrmcpy(char *orig, size_t size)
{
    char *res = xmalloc(size+1);

    strncpy(res, orig, size);
    res[size] = 0;
    return res;
}


/*
 * convert a human-readable number to an appropriately scaled float
 */
float
normalize(float num, char suffix)
{
    switch ( tolower(suffix) ) {
    case 'k': return num * 1000.0;
    case 'm': return num * 1000.0 * 1000.0;
    case 'g': return num * 1000.0 * 1000.0 * 1000.0;
    case 't': return num * 1000.0 * 1000.0 * 1000.0 * 1000.0;
    }
    return num;
}


/*
 * sort two lines from du -sh
 */
int
sorth(const void *a, const void *b)
{
    float sz_a, sz_b;
    char mk_a, mk_b;
    int ret_a, ret_b;

    char *s_a = *(char**)a;
    char *s_b = *(char**)b;

    /* sort nulls by if A == null return 1 else if B == null return -1
     */
    if ( s_a == 0 )
	return -1;
    else if ( s_b == 0 )
	return 1;

    ret_a = sscanf(s_a, "%f%c", &sz_a, &mk_a);
    ret_b = sscanf(s_b, "%f%c", &sz_b, &mk_b);

    /* lines that aren't from du -sh sort to the start of the list
     */
    if ( ret_a < 1 )
	return (ret_b > 0) ? -1 : strcasecmp(s_a, s_b);
    else if ( ret_b < 1 )
	return 1;

    if ( ret_a == 2) sz_a = normalize(sz_a, mk_a);
    if ( ret_b == 2) sz_b = normalize(sz_b, mk_b);

    if ( sz_a < sz_b )
	return -1;
    else if ( sz_a > sz_b )
	return 1;

    return 0;
}


#ifndef HAS_FGETLN

char *input_line = NULL;
int input_line_sz = 0;

char *
fgetln(FILE *input, size_t *ret)
{
    int c;
    int idx = 0;

    if ( ferror(input) || feof(input) )
	return NULL;

    while ( (c=fgetc(input)) != EOF ) {
	if ( idx >= input_line_sz ) {
	    input_line_sz = (input_line_sz == 0) ? 80 : (2*input_line_sz);
	    input_line = input_line ? xrealloc(input_line, input_line_sz * sizeof (*input_line))
				    : xmalloc(input_line_sz * sizeof (*input_line));
	}
	input_line[idx++] = c;

	if ( c == '\n')
	    break;
    }
    (*ret) = idx;
    return input_line;
}

#endif



int
main(argc, argv)
char **argv;
{
    char *thisline;
    size_t size;
    int ct = 0;
    int i;

    if ( (argc > 1) && (strcmp(argv[1], "-r") == 0) )
	reversed=1;
    
    lines = xmalloc(szlines * sizeof(*lines));

    while (( thisline = fgetln(stdin, &size) )) {
	ct++;
	if ( (size > 0) && (thisline[size-1] == '\n') )
	    --size;
	if ( size < 1 ) 	/* don't sort empty lines */
	    continue;
	if ( nrlines >= szlines ) {
	    szlines *= 2;
	    lines = xrealloc(lines, szlines * sizeof(*lines));
	}
	lines[nrlines++] = xstrmcpy(thisline, size);
    }
    if ( nrlines > 1 )
	qsort(lines, nrlines, sizeof(*lines), sorth);

    if ( reversed ) {
	for (i=nrlines-1; i >= 0; --i)
	    if ( lines[i] )
		puts(lines[i]);
    }
    else {
	for (i=0; i < nrlines; i++)
	    if ( lines[i] )
		puts(lines[i]);
    }
    
    return 0;
}
