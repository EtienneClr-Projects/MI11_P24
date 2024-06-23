/*----------------------------------------------------------------------------*
 * fichier : noyau_test.c                                                     *
 * programme de test du noyaut                                                *
 *----------------------------------------------------------------------------*/
/*
ouvrier indépendant
FONCTIONS PERIODIQUES (par ordre de prio)
- pause déjeuner (tous les 24h pendant 1h) debut 12h
- réunion de chantier (tous les 3*24h pendant 2h) debut 9h
- travailler sur le chantier (tous les 24h pendant 4h) debut 8h
- controler les outils (met en panne random) (tous les 24h pendant 1h) debut 13h
- repos (tous les 24h pendant 8h) debut 22h
- checker la boite mail (tous les 24h pendant 1h) debut 14h (peut ajouter la tache faire un devis, faire de l'administratif)

FONCTIONS APERIODIQUES (par ordre de prio)
- acheter des outils (si variable en panne) 1h
- faire l'administratif (3h)
- faire des devis (2h)
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stm32h7xx.h"
#include "serialio.h"
#include "noyau_prio.h"
#include "noyau_file_prio.h"
#include "delay.h"
#include "TERMINAL.h"


uint16_t pos_x = 1;
uint16_t pos_y = 10;
#define MAX_CARA_LIGNE 190
#define POS_CHRONO 10

typedef struct
{
	// temps de la tache
	uint16_t Nb_tour;
	// temps entre deux taches
	uint16_t wait_time;
} NOYAU_TCB_ADD;

void init_P_and_fond(void);
TACHE tachePeriodique(void);
TACHE tacheAPeriodique(TACHE_APERIODIC *tache);
TACHE tachedefond(void);

NOYAU_TCB_ADD _noyau_tcb_add[MAX_TACHES_NOYAU];

#define ID_AP_ACHETER_OUTILS 0
#define ID_AP_ADMINISTRATIF 1
#define ID_AP_DEVIS 2

#define ID_P_PAUSE_DEJEUNER 1			// premiere liste de 0 à 7
#define ID_P_REUNION_CHANTIER 8			// deuxieme liste de 8 à 15
#define ID_P_TRAVAILLER_SUR_CHANTIER 16 // troisieme liste de 16 à 23
#define ID_P_CONTROLER_OUTILS 17		// troisieme liste de 16 à 23
#define ID_P_REPOS 24					// quatrieme liste de 24 à 31
#define ID_P_CHECKER_MAIL 25			// quatrieme liste de 24 à 31

#define ID_TACHE_DE_FOND 63

int already_executed = 0;

void init_P_and_fond(void)
{
	_lock_();
	if (already_executed == 0)
	{
		// *************************
		// TACHES PERIODIQUES
		// *************************
		uint16_t id;

		// PAUSE DEJEUNER
		id = ID_P_PAUSE_DEJEUNER;
		_noyau_tcb_add[id].Nb_tour = 1;		 // equivalent 1h
		_noyau_tcb_add[id].wait_time = 1000; // equivalent 24h
		active(cree((TACHE_ADR)tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

		// REUNION DE CHANTIER
		id = ID_P_REUNION_CHANTIER;
		_noyau_tcb_add[id].Nb_tour = 2;
		_noyau_tcb_add[id].wait_time = 300;
		active(cree((TACHE_ADR)tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

		// TRAVAILLER SUR LE CHANTIER
		id = ID_P_TRAVAILLER_SUR_CHANTIER;
		_noyau_tcb_add[id].Nb_tour = 4;
		_noyau_tcb_add[id].wait_time = 300;
		active(cree((TACHE_ADR)tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

		// CONTROLER LES OUTILS
		id = ID_P_CONTROLER_OUTILS;
		_noyau_tcb_add[id].Nb_tour = 1;
		_noyau_tcb_add[id].wait_time = 300;
		active(cree((TACHE_ADR)tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

		// REPOS
		id = ID_P_REPOS;
		_noyau_tcb_add[id].Nb_tour = 8;
		_noyau_tcb_add[id].wait_time = 300;
		active(cree((TACHE_ADR)tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

		// CHECKER LA BOITE MAIL
		id = ID_P_CHECKER_MAIL;
		_noyau_tcb_add[id].Nb_tour = 1;
		_noyau_tcb_add[id].wait_time = 1000;
		active(cree((TACHE_ADR)tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

		// LANCER LA TACHE DE FOND
		id = ID_TACHE_DE_FOND; // quatrieme liste de 24 à 31
		_noyau_tcb_add[id].Nb_tour = 1;
		_noyau_tcb_add[id].wait_time = 60;
		active(cree((TACHE_ADR)tachedefond, id, (void *)&_noyau_tcb_add[id]));
		printf("on a créé la tache de fond\n");
	}

	already_executed = 1;
	_unlock_();
}

int main()
{
	usart_init(115200);
	CLEAR_SCREEN(1);
	SET_CURSOR_POSITION(1, 1);
	test_colors();
	CLEAR_SCREEN(1);
	SET_CURSOR_POSITION(1, 1);
	puts("Test noyau");
	puts("Noyau preemptif");
	SET_CURSOR_POSITION(3, 1);
	SAVE_CURSOR_POSITION();

	start((TACHE_ADR)init_P_and_fond);

	// #########################################################
	// ##################### INIT APERIODIC ####################
	// #########################################################
	// INIT FIFO
	fifo_init(&fifo_tache_aperiodic);

	uint16_t id;

	// ACHETER DES OUTILS
	id = ID_AP_ACHETER_OUTILS;
	tab_tache_aperiodic[id].adr = (TACHE_ADR)tacheAPeriodique;
	tab_tache_aperiodic[id].name = "ACHETER DES OUTILS";
	char chaine[24] = "J'ai achete des outils !";
	for (int j = 0; j < 25; j++)
	{
		tab_tache_aperiodic[id].params[j] = chaine[j];
	}
	active_aperiodic(cree_aperiodic((TACHE_ADR)tacheAPeriodique, id, NULL));
	fifo_ajoute(&fifo_tache_aperiodic, id);

	// FAIRE L'ADMINISTRATIF
	id = ID_AP_ADMINISTRATIF;
	tab_tache_aperiodic[id].adr = (TACHE_ADR)tacheAPeriodique;
	tab_tache_aperiodic[id].name = "FAIRE L'ADMINISTRATIF";
	char chaine2[24] = "J'ai fait de l'admin !";
	for (int j = 0; j < 25; j++)
	{
		tab_tache_aperiodic[id].params[j] = chaine2[j];
	}
	active_aperiodic(cree_aperiodic((TACHE_ADR)tacheAPeriodique, id, NULL));
	fifo_ajoute(&fifo_tache_aperiodic, id);

	// FAIRE DES DEVIS
	id = ID_AP_DEVIS;
	tab_tache_aperiodic[id].adr = (TACHE_ADR)tacheAPeriodique;
	tab_tache_aperiodic[id].name = "FAIRE DES DEVIS";
	char chaine3[24] = "J'ai fait des devis !";
	for (int j = 0; j < 25; j++)
	{
		tab_tache_aperiodic[id].params[j] = chaine3[j];
	}
	active_aperiodic(cree_aperiodic((TACHE_ADR)tacheAPeriodique, id, NULL));
	fifo_ajoute(&fifo_tache_aperiodic, id);

	// #########################################################
	// ############ INIT PERIODIC & TACHE DE FOND ##############
	// #########################################################
	init_P_and_fond();

	schedule();
	while (1)
	{
	}
	return (0);
}

int count_before_acheter_outils = 5;

TACHE tachePeriodique(void)
{
	volatile NOYAU_TCB *p_tcb = NULL;
	volatile uint16_t id_tache;
	uint16_t i, j = 1;

	id_tache = noyau_get_tc();
	p_tcb = noyau_get_p_tcb(id_tache);

	volatile uint16_t Nb_tour = ((NOYAU_TCB_ADD *)(p_tcb->tcb_add))->Nb_tour;
	volatile uint16_t wait_time = ((NOYAU_TCB_ADD *)(p_tcb->tcb_add))->wait_time;

	// on laisse du temps à la tâche de fond de démarrer toutes les tâches
	delay_n_ticks(20);
	while (1)
	{
		// id_tache = noyau_get_tc();
		while (tache_get_flag_tick(id_tache) != 0)
		{
			_lock_();

			// tous les 5 controles, on doit acheter des outils
			if (id_tache == ID_P_CONTROLER_OUTILS)
			{
				if (count_before_acheter_outils == 0)
				{
					// ACHETER DES OUTILS
					fifo_ajoute(&fifo_tache_aperiodic, 0);
					count_before_acheter_outils = 5;
				}
				else
				{
					count_before_acheter_outils--;
				}
			}

			for (i = POS_CHRONO; i < (POS_CHRONO + 8); i++)
			{
				printf("%s%d;%d%s", CODE_ESCAPE_BASE, i, pos_x, "H");
				if ((i - POS_CHRONO) == (id_tache >> 3))
				{
					SET_CURSOR_POSITION(i, pos_x);
					SET_BACKGROUND_COLOR(id_tache + 16);
					SET_FONT_COLOR(15);
					printf("%2d", id_tache);
					SET_BACKGROUND_COLOR(0);

					SET_CURSOR_POSITION(20, pos_x);
					printf("     ");
				}
				else
				{
					SET_BACKGROUND_COLOR(0);
					printf("  ");
				}
			}
			pos_x = pos_x + 2;
			if (pos_x > MAX_CARA_LIGNE)
			{
				pos_x = 1;
			}

			if (j >= Nb_tour)
			{
				j = 1;
				delay_n_ticks(wait_time);
			}
			else
			{
				j++;
			}
			_unlock_();
			tache_reset_flag_tick(id_tache);
		}
	}
}

TACHE tachedefond(void) // ordonnanceur des taches aperiodiques
{
	while (1)
	{
		// ici on execute les taches aperiodiques dans l'ordre de passage FIFO
		uint8_t index_to_execute;
		if (fifo_retire(&fifo_tache_aperiodic, (uint8_t *)&index_to_execute) == -1)
		{
			// exec
			((TACHE_AP_ADR)tab_tache_aperiodic[index_to_execute].adr)(&tab_tache_aperiodic[index_to_execute]);
		}
		else // FIFO empty
		{
			//			printf("fifo empty \n");
		}
	}
}

TACHE tacheAPeriodique(TACHE_APERIODIC *tache)
{
	// SET_CURSOR_POSITION(21,0);
	// SET_BACKGROUND_COLOR(1);
	// printf("debut TACHE AP : %s                        ",tache->name);

	SET_CURSOR_POSITION(25, 0);
	printf("                                ");

	int finished = 0;
	int i = 0;
	while (finished != 1)
	{
		_lock_();

		// ici tache->params est un void* donc on doit caster en char*
		char *param_string = tache->params;
		char chaine[25];
		for (int j = 0; j < 25; j++)
		{
			chaine[j] = param_string[j];
		}

		for (int j = 0; j < i; j++)
		{
			SET_CURSOR_POSITION(25, i);
			printf("%c", chaine[j]);
		}
		if ((tache->params)[i] == '!')
		{ // caractère de fin
			finished = 1;
		};

		for (int j = 0; j < 8; j++)
		{
			SET_CURSOR_POSITION(10 + j, pos_x);
			SET_BACKGROUND_COLOR(0);
			printf("  ");
		}
		SET_CURSOR_POSITION(20, pos_x);
		SET_BACKGROUND_COLOR(2);
		SET_FONT_COLOR(15);
		printf("##");
		SET_BACKGROUND_COLOR(0);
		pos_x = pos_x + 2;
		if (pos_x > MAX_CARA_LIGNE)
		{
			pos_x = 1;
		}

		i++;
		_unlock_();
		for (int j = 0; j < 1000000; j++)
		{
		} // attente active
	};
	//    SET_CURSOR_POSITION(22,0);
	//    SET_BACKGROUND_COLOR(2);
	//    printf("fin   TACHE AP : %s                        ",tache->name);
}
