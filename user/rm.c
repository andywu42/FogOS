#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define MAX_PATH 512

// Boolean variables for flag processing, defaulted to false
int v, r, f, i, d = 0;

/*
	Deletes files/directories. Can verbose

	@param path path to delete
*/
void 
delete(char* path)
{
  if(unlink(path) < 0) {
  	if (!f) {
  	  fprintf(2, "rm: %s failed to delete\n", path);
  	  exit(1);
  	}
  }
  
  if (v) {
  	printf("rm: deleted %s\n", path);
  }
}

/*
	Helper function to concat two string paths together to a destination string

	@param dest destination string
	@param path1 first path
	@param path2 second path
*/
void
concat_paths(char* dest, char* path1, char* path2) {
	int i = 0, j = 0;

	while (path1[i] != '\0' && i < MAX_PATH - 1) {
		dest[i] = path1[i];
		i++;
	}

	if (i > 0 && dest[i-1] != '/') {
		dest[i] = '/';
		i++;
	}

	while (path2[j] != '\0' && i < MAX_PATH - 1) {
		dest[i] = path2[j];
		i++;
		j++;
	}

	dest[i] = '\0';
}

/*
	Main function of rm. Checks for flags and appropriately rm
	files/directories using delete(). Possible flags include:
	-r recursive
	-f force
	-i interactive
	-d empty directory

	@param path path to process
*/
void
rm(char *path)
{
	// Cannot rm an empty directory recursively
	if (d && r) {
		fprintf(2, "rm: Cannot use flags '-d' and '-r' together.\n");
		exit(1);
	}

	// Cannot rm with interactive and force
	if (i && f) {
		fprintf(2, "rm: Cannot use flags '-i' and '-f' together.\n");
		exit(1);
	}

	// Interactive
	if (i) {
		while (1) {
			printf("rm: Delete '%s'?\nEnter [y] for yes or [n] for no\n", path);
			char answer[32];
			read(0, answer, 32 - 1);
			answer[31] = '\0';
			if (answer[0] == 'y') {
				break;
			} else if (answer[0] == 'n') {
				return;
			} else {
				printf("Invalid answer.\n");
			}
		}
	}

	// Empty directory, just delete it
	if (d) {
		delete(path);
		return;
	}

	// Directory processing
	struct stat st;
	if (stat(path, &st) < 0) {
		fprintf(2, "rm: stat(%s) failed\n", path);
	}
	if (st.type == T_DIR) {
		// Recursively delete all files/directories
		if (r) {
			int dir = open(path, 0);
			if (dir < 0) {
				fprintf(2, "rm: open(%s) error\n", path);
				exit(1);
			}
			struct dirent de;
			int n;
			while ((n = read(dir, &de, sizeof(de))) > 0) {
				if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
					continue;	// Skip empty entries and . or ..
				}
				char entry[512];
				concat_paths(entry, path, de.name);
				rm(entry);
			}
			close(dir);
		}
	} 
	delete(path);
}


int
main(int argc, char *argv[])
{
  int index;
  int start = 1;

  if(argc < 2){
    fprintf(2, "Usage: rm files...\n");
    exit(1);
  }

  // Parse the flags
  for (index = 1; index < argc; index++) {
  	if (argv[index][0] == '-') {	// Check if argv is even a flag
  		char *flag = argv[index] + 1;
  		while (*flag) {
  			switch (*flag) {
  			  	case 'r':
  			  		r = 1;
  			  		break;
  			  	case 'v':
  			  		v = 1;
  			  		break;
  			  	case 'f':
  			  		f = 1;
  			  		break;
  			  	case 'i':
  			  		i = 1;
  			  		break;
  			  	case 'd':
  			  		d = 1;
  			  		break;
  			  	default:
  			  		fprintf(2, "rm: invalid flag '%s'\n", *flag);
  			  		exit(1);
  			  	}
  			flag++;
  		}
  	} else {	// Not a flag
		start = index;
  		break;
  	}
  }
  
  // Parse the files/directories
  for (index = start; index < argc; index++) {
  	rm(argv[index]);
  }

  exit(0);
}
