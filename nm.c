#include <stdio.h>
#include <sys/mman.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

// /usr/include/mach-o/loader.h
// /usr/include/mach-o/nlist.h

//hexdump -C -n 4 nm

// N_UNDF
// N_DATA
// N_TEXT
// N_FN

char		getType(uint8_t type, uint8_t sect)
{
	if (type == 1)
		return ('U');
	if (type == 14)
	{
		if (sect == 1)
			return ('t');
		if (sect == 8)
			return ('s');
		if (sect == 9)
			return ('d');
		if (sect == 11)
			return ('b');
	}
	if (type == 15)
	{
		if (sect == 1)
			return ('T');
		if (sect == 8)
			return ('D');
		if (sect == 9 || sect == 11)
			return ('S');
		if (sect == 10)
			return ('D');
	}
	return ('X');
}

void		print_output(int nsyms, int symoff, int stroff, char *ptr)
{
	int				i;
	char			*strtable;
	struct nlist_64 *el;

	el = (void *)ptr + symoff;
	strtable = ptr + stroff;

	for (i =0; i < nsyms; ++i)
		printf("%x       %c      %s\n", el[i].n_value, getType(el[i].n_type, el[i].n_sect), strtable + el[i].n_un.n_strx);
}

void		handle_64(char *ptr)
{
	int		ncmds;
	int						i;
	struct mach_header_64	*header;
	struct load_command		*lc;
	struct symtab_command	*sym;


	header = (struct mach_header_64 *)ptr;
	ncmds = header->ncmds;
	lc = (void *) ptr + sizeof(*header);
	for (i = 0 ; i < ncmds; ++i)
	{
		if (lc->cmd == LC_SYMTAB)
		{
			sym = (struct symtab_command *) lc;
			print_output(sym->nsyms, sym->symoff, sym->stroff, ptr);
			break ;
		}
		lc = (void *) lc + lc->cmdsize;
	}
}

void		nm(char *ptr)
{
	int magic_number;

	magic_number = *(int *) ptr;
	if (magic_number == MH_MAGIC_64)
		handle_64(ptr);
}

int			main(int ac, char **av)
{
	int		fd;
	char	*ptr;
	struct 	stat buf;

	if (ac != 2)
	{
		fprintf(stderr, "Needs an arg.\n");
		return (EXIT_FAILURE);
	}

	if ((fd = open(av[1], O_RDONLY)) < 0)
	{
		perror("open");
		return (EXIT_FAILURE);
	}

	if (fstat(fd, &buf) < 0)
	{
		perror("fstat");
		return (EXIT_FAILURE);
	}

	if ((ptr = mmap(0, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
	{
		perror("mmap");
		return (EXIT_FAILURE);
	}

	nm(ptr);
	if (munmap(ptr, buf.st_size) < 0)
	{
		perror("munmap");
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
