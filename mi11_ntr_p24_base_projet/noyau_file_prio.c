/*----------------------------------------------------------------------------*
 * fichier : noyau_file_prio.c                                                *
 * gestion de la file d'attente des taches pretes et actives                  *
 * la file est rangee dans un tableau. ce fichier definit toutes              *
 * les primitives de base                                                     *
 *----------------------------------------------------------------------------*/


#include <stdint.h>
#include "noyau_file_prio.h"
// recuperation du bon fichier selon l'architecture pour la fonction printf
#include "serialio.h"


/*----------------------------------------------------------------------------*
 * variables communes a toutes les procedures                                 *
 *----------------------------------------------------------------------------*/

/*
 * tableau qui stocke les taches
 * indice = numero de tache
 * valeur = tache suivante
 */
static uint16_t _file[MAX_FILE][MAX_TACHES_FILE];

/*
 * index de queue
 * valeur de l'index de la tache en cours d'execution
 * pointe sur la prochaine tache a activer
 */
static uint16_t _queue[MAX_FILE];

static uint8_t  _file_u;

const uint16_t  tab_num_file[256]=  { 8, 0, 1, 0, 2, 0, 1, 0, //   0 to 7
									  3, 0, 1, 0, 2, 0, 1, 0, //   8 to 15
									  4, 0, 1, 0, 2, 0, 1, 0, //  16 to 23
									  3, 0, 1, 0, 2, 0, 1, 0, //  24 to 31
									  5, 0, 1, 0, 2, 0, 1, 0, //  32 to 39
									  3, 0, 1, 0, 2, 0, 1, 0, //  40 to 47
									  4, 0, 1, 0, 2, 0, 1, 0, //  48 to 55
									  3, 0, 1, 0, 2, 0, 1, 0, //  56 to 63
									  6, 0, 1, 0, 2, 0, 1, 0, //  64 to 71
									  3, 0, 1, 0, 2, 0, 1, 0, //  72 to 79
									  4, 0, 1, 0, 2, 0, 1, 0, //  80 to 87
									  3, 0, 1, 0, 2, 0, 1, 0, //  88 to 95
									  5, 0, 1, 0, 2, 0, 1, 0, //  96 to 103
									  3, 0, 1, 0, 2, 0, 1, 0, // 104 to 111
									  4, 0, 1, 0, 2, 0, 1, 0, // 112 to 119
									  3, 0, 1, 0, 2, 0, 1, 0, // 120 to 127
									  7, 0, 1, 0, 2, 0, 1, 0, // 128 to 135
									  3, 0, 1, 0, 2, 0, 1, 0, // 136 to 143
									  4, 0, 1, 0, 2, 0, 1, 0, // 144 to 151
									  3, 0, 1, 0, 2, 0, 1, 0, // 152 to 159
									  5, 0, 1, 0, 2, 0, 1, 0, // 160 to 167
									  3, 0, 1, 0, 2, 0, 1, 0, // 168 to 175
									  4, 0, 1, 0, 2, 0, 1, 0, // 176 to 183
									  3, 0, 1, 0, 2, 0, 1, 0, // 184 to 191
									  6, 0, 1, 0, 2, 0, 1, 0, // 192 to 199
									  3, 0, 1, 0, 2, 0, 1, 0, // 200 to 207
									  4, 0, 1, 0, 2, 0, 1, 0, // 208 to 215
									  3, 0, 1, 0, 2, 0, 1, 0, // 216 to 223
									  5, 0, 1, 0, 2, 0, 1, 0, // 224 to 231
									  3, 0, 1, 0, 2, 0, 1, 0, // 232 to 239
									  4, 0, 1, 0, 2, 0, 1, 0, // 240 to 247
									  3, 0, 1, 0, 2, 0, 1, 0}; // 248 to 255


/*----------------------------------------------------------------------------*
 * fonctions de gestion de la file                                            *
 *----------------------------------------------------------------------------*/

/*
 * initialise la file
 * entre  : sans
 * sortie : sans
 * description : la queue est initialisee à une valeur de tache impossible
 */
void file_init(void) {
	uint16_t i;

	for (i=0; i<MAX_FILE; i++){
		_queue[i] = F_VIDE;
	}
	_file_u = 0;
}

/*
 * ajoute une tache dans la file
 * entre  : n numero de la tache a ajouter
 * sortie : sans
 * description : ajoute la tache n en fin de file
 */
void file_ajoute(uint16_t n) {
	uint16_t num_file, num_t, *q, *f;

	num_file = (n & MASK_NF)>>3;
	num_t    = n & MASK_NT;
	q = &_queue[num_file];
	f = &_file[num_file][0];
    if (*q == F_VIDE) {
        f[num_t] = n;
        _file_u = _file_u | (1u << num_file);
    } else {
        f[num_t] = f[*q];
        f[*q] = n;
    }

    *q = num_t;
}

/*
 * retire une tache de la file
 * entre  : t numero de la tache a retirer
 * sortie : sans
 * description : retire la tache t de la file. L'ordre de la file n'est pas
                 modifie
 */
void file_retire(uint16_t t) {
	uint16_t num_file, num_t, *q, *f;

	num_file = (t & MASK_NF)>>3;
	num_t    = t & MASK_NT;
	q = &_queue[num_file];
	f = &_file[num_file][0];

    if (*q == (f[*q] & MASK_NT)) {
    	_file_u = _file_u & (~(1u << num_file));
        *q = F_VIDE;
    } else {
        if (num_t == *q) {
            *q = f[*q] & MASK_NT;
            while (f[*q] != t) {
                *q = f[*q] & MASK_NT;
            }
            f[*q] = f[num_t];
        } else {
            while (f[*q] != t) {
                *q = f[*q] & MASK_NT;
            }
            f[*q] = f[f[*q] & MASK_NT];
        }
    }
}

/*
 * recherche la tache suivante a executer
 * entre  : sans
 * sortie : numero de la tache a activer
 * description : queue pointe sur la tache suivante
 */
uint16_t file_suivant(void) {
	uint16_t num_file, *q, *f, id;

	num_file = tab_num_file[_file_u];
	q = &_queue[num_file];
	f = &_file[num_file][0];

    if (*q != F_VIDE) {
    	id = f[*q];
        *q = id & MASK_NT;
        return (id);
    }
    return (-1);
}

/*
 * affiche la queue, donc la derniere tache
 * entre  : sans
 * sortie : sans
 * description : affiche la valeur de queue
 */
void file_affiche_queue() {
	uint16_t i;
	for (i=0; i<MAX_FILE; i++){
		 printf("_queue[%d] = %d\n", i, _queue[i]);
	}
}

/*
 * affiche la file
 * entre  : sans
 * sortie : sans
 * description : affiche les valeurs de la file
 */
void file_affiche() {
	uint16_t i,j;

    for (j=0; j<MAX_FILE; j++){
		printf("Tache   | ");
		for (i = 0; i < MAX_TACHES_FILE; i++) {
			printf("%03d | ", i);
		}

		printf("\nSuivant | ");
		for (i = 0; i < MAX_TACHES_FILE; i++) {
			printf("%03d | ", _file[j][i]);
		}
		printf("\n");
    }
}
