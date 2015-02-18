

// 				    9c 9d 9e 9f a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa
// 0000000100000a9b 55 48 89 e5 89 fa 89 f0 88 55 fc 88 45 f8 80 7d
// 0000000100000aab fc 01 75 07 b8 55 00 00 00 eb 7f 80 7d fc 0e 75
// ...

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

// void		print_output(int nsyms, int symoff, int stroff, char *ptr)
// {
// 	int				i;
// 	char			*strtable;
// 	struct nlist_64 *el;

// 	el = (void *)ptr + symoff;
// 	strtable = ptr + stroff;

// 	for (i =0; i < nsyms; ++i)
// 		printf("%x       %c      %s\n", el[i].n_value, getType(el[i].n_type, el[i].n_sect), strtable + el[i].n_un.n_strx);
// }


int		ft_strequ(char const *s1, char const *s2)
{
	int		i;

	if (s1 == NULL || s2 == NULL)
		return (0);
	i = 0;
	while (s1[i] != '\0' && s2[i] != '\0' && s1[i] == s2[i])
		i++;
	if (s1[i] == '\0' && s2[i] == '\0')
		return (1);
	return (0);
}

void		handle_64(char *ptr)
{
	int						ncmds;
	int						i;
	struct mach_header_64	*header;
	struct load_command		*lc;
	struct segment_command_64		*seg_cmd;


	header = (struct mach_header_64 *)ptr;
	ncmds = header->ncmds;
	lc = (void *) ptr + sizeof(*header);
	for (i = 0 ; i < ncmds; ++i)
	{
		if (lc->cmd == LC_SEGMENT_64)
		{
			seg_cmd = (struct segment_command_64 *) lc;
			if (ft_strequ(seg_cmd->segname, "__TEXT") && seg_cmd->nsects)
				printf("%s %x\n", seg_cmd->segname, seg_cmd->vmaddr);
			// print_output(sym->nsyms, sym->symoff, sym->stroff, ptr);
			// break ;
		}
		lc = (void *) lc + lc->cmdsize;
	}
}

void		ft_otool(char *ptr)
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

	ft_otool(ptr);
	if (munmap(ptr, buf.st_size) < 0)
	{
		perror("munmap");
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
