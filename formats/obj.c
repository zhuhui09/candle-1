#include "obj.h"
#include <candle.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ////////////////////////////////////////////////////////////////////////// */
/*                                .OBJ LOADER                                 */
/* ////////////////////////////////////////////////////////////////////////// */

static char* obj__strsep(char** stringp, const char* delim)
{
	char* start = *stringp;
	char* p;

	p = (start != NULL) ? strpbrk(start, delim) : NULL;

	if (p == NULL)
	{
		*stringp = NULL;
	}
	else
	{
		*p = '\0';
		*stringp = p + 1;
	}

	return start;
}

struct vert {
	int v, t, n;
};
struct face {
	int nv;
	union {
		struct vert v[4];
		int l[12];
	} value;
};

static void ignoreLine(FILE *fp)
{
    char ch;
    while((ch=fgetc(fp)) != EOF && ch!='\n');
}

static int isToIgnore(char c)
{
	/* return c == '#' || c == 'u' || c == 'o' || c == 's' || c == 'm' ||
	 * c == 'g'; */
	return c != 'v' && c != 'f';
}


static void count(FILE *fp, int *numV, int *numVT, int *numVN, int *numF)
{
    char ch;
    (*numV) = 0;
    (*numVT) = 0;
    (*numVN) = 0;
    (*numF) = 0;
    rewind(fp);
    while((ch = fgetc(fp)) != EOF )
    {
        if (isToIgnore(ch))
		{
			ignoreLine(fp);
		}
        else
		{
			switch (ch)
			{
				case 'v':
					if ((ch = fgetc(fp)) == 't')
						(*numVT)++;
					else if (ch == 'n')
						(*numVN)++;
					else (*numV)++;
					break;
				case 'f':(*numF)++;
			}
		}
    }
    rewind(fp);
}

static void read_prop(mesh_t *self, FILE * fp, vec3_t *tempNorm,
		vec2_t *tempText, struct face *tempFace)
{
    int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
    char ch;
	int ret;

    rewind(fp);
    while((ch = fgetc(fp)) != EOF)
    {
        if (isToIgnore(ch))
		{
			ignoreLine(fp);
		}
        else
		{
			struct face *tf;
			char *line_i, *words, *token;

			char *line = malloc(512);
			int i;
			switch(ch)
			{
				case 'v':
					if ((ch=fgetc(fp))=='t')
					{
						fgetc(fp);
						ret = fscanf(fp, "%f", &(tempText[n1].x)); fgetc(fp);
						if (!ret) exit(1);
						ret = fscanf(fp, "%f", &(tempText[n1].y)); fgetc(fp);
						if (!ret) exit(1);
						n1++;
					}
					else if (ch=='n')
					{
						fgetc(fp);
						ret = fscanf(fp, "%f", &(tempNorm[n2].x)); fgetc(fp);
						if (!ret) exit(1);
						ret = fscanf(fp, "%f", &(tempNorm[n2].y)); fgetc(fp);
						if (!ret) exit(1);
						ret = fscanf(fp, "%f", &(tempNorm[n2].z)); fgetc(fp);
						if (!ret) exit(1);
						n2++;
					}
					else
					{
						vec3_t pos;
						ret = fscanf(fp, "%f", &pos.x); fgetc(fp);
						if (!ret) exit(1);
						ret = fscanf(fp, "%f", &pos.y); fgetc(fp);
						if (!ret) exit(1);
						ret = fscanf(fp, "%f", &pos.z); fgetc(fp);
						mesh_add_vert(self, VEC3(pos.x, pos.y, pos.z));
						if (!ret) exit(1);
						n3++;
					}
					break;
				case 'f':
					if (fgets(line, 512, fp) == NULL) exit(1);
					tf = &tempFace[n4];
					line_i = line;
					for (i = 0; (words = obj__strsep(&line_i, " ")) != NULL; i += 3)
					{
						int j;
						char *words_i = words;
						if (words[0] == '\0')
						{
							i -= 3;
							continue;
						}
						for (j = 0;(token = obj__strsep(&words_i, "/")) != NULL; j++)
						{
							if ((token[0] < '0' || token[0] > '9') ||
									sscanf(token, "%d", &(tf->value.l[i + j])) <= 0)
							{
								tf->value.l[i + j] = 0;
							}
							tf->value.l[i + j]--;
						}
					}
					tf->nv = i / 3;

					n4++;
			}
			free(line);
		}
    }
}

void mesh_load_obj(mesh_t *self, const char *filename)
{
	FILE *fp;
    int i;
    int v, vt, vn, f;
	vec3_t *tempNorm;
	vec2_t *tempText;
	struct face *tempFace;

	strncpy(self->name, filename, sizeof(self->name) - 1);
    fp = fopen(filename, "r");
	if (!fp)
	{
		printf("Could not find object '%s'\n", filename);
		return;
	}

    count(fp, &v, &vt, &vn, &f);
    tempNorm = malloc((vn + 1) * sizeof(vec3_t));
    tempText = malloc((vt + 1) * sizeof(vec2_t));
    tempFace = malloc(f * sizeof(struct face));

	tempNorm[0] = Z3;
	tempText[0] = Z2;
    read_prop(self, fp, &tempNorm[1], &tempText[1], tempFace);


	for (i = 0; i < f; i++)
	{
		struct face *face = &tempFace[i];
		if (face->nv == 3)
		{
			mesh_add_triangle(self,
					face->value.v[0].v, tempNorm[face->value.v[0].n + 1], tempText[face->value.v[0].t + 1],
					face->value.v[1].v, tempNorm[face->value.v[1].n + 1], tempText[face->value.v[1].t + 1],
					face->value.v[2].v, tempNorm[face->value.v[2].n + 1], tempText[face->value.v[2].t + 1]);
		}
		else if (face->nv == 4)
		{
			mesh_add_quad(self,
					face->value.v[0].v, tempNorm[face->value.v[0].n + 1], tempText[face->value.v[0].t + 1],
					face->value.v[1].v, tempNorm[face->value.v[1].n + 1], tempText[face->value.v[1].t + 1],
					face->value.v[2].v, tempNorm[face->value.v[2].n + 1], tempText[face->value.v[2].t + 1],
					face->value.v[3].v, tempNorm[face->value.v[3].n + 1], tempText[face->value.v[3].t + 1]);
		}
	}

	free(tempNorm);
	free(tempText);
    free(tempFace);

    fclose(fp);

}


