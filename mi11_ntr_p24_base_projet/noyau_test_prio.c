/*----------------------------------------------------------------------------*
 * fichier : noyau_test.c                                                     *
 * programme de test du noyaut                                                *
 *----------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>

#include "stm32h7xx.h"
#include "serialio.h"
#include "noyau_prio.h"
#include "noyau_file_prio.h"
#include "delay.h"
#include "TERMINAL.h"


uint16_t pos_x = 1;
uint16_t pos_y = 10;

typedef struct {
	// temps de la tache
	uint16_t Nb_tour;
	// temps entre deux taches
	uint16_t wait_time;
} NOYAU_TCB_ADD;

TACHE tachedefond(void);
TACHE tachePeriodique(void);

#define MAX_CARA_LIGNE 80
#define POS_CHRONO 10

NOYAU_TCB_ADD _noyau_tcb_add[MAX_TACHES_NOYAU];


int main()
{
	usart_init(115200);
	CLEAR_SCREEN(1);
    SET_CURSOR_POSITION(1,1);
    test_colors();
    CLEAR_SCREEN(1);
    SET_CURSOR_POSITION(1,1);
    puts("Test noyau");
    puts("Noyau preemptif");
    SET_CURSOR_POSITION(5,1);
    SAVE_CURSOR_POSITION();
	
	
    uint16_t id;
// *************************
// TACHES PERIODIQUES
// *************************
    //PAUSE DEJEUNER
	id = 1; // premiere liste de 0 à 7
    _noyau_tcb_add[id].Nb_tour = 1; // equivalent 1h
    _noyau_tcb_add[id].wait_time = 1000; // equivalent 24h
    active(cree(tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

    //REUNION DE CHANTIER
    id = 8; // deuxieme liste de 8 à 15
    _noyau_tcb_add[id].Nb_tour = 2;
    _noyau_tcb_add[id].wait_time = 3000;
    active(cree(tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

    //TRAVAILLER SUR LE CHANTIER
    id = 16; // troisieme liste de 16 à 23
    _noyau_tcb_add[id].Nb_tour = 4;
    _noyau_tcb_add[id].wait_time = 1000;
    active(cree(tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

    //CONTROLER LES OUTILS
    id = 17; // troisieme liste de 16 à 23
    _noyau_tcb_add[id].Nb_tour = 1;
    _noyau_tcb_add[id].wait_time = 1000;
    active(cree(tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

    //REPOS
    id = 24; // quatrieme liste de 24 à 31
    _noyau_tcb_add[id].Nb_tour = 8;
    _noyau_tcb_add[id].wait_time = 1000;
    active(cree(tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

    //CHECKER LA BOITE MAIL
    id = 25; // quatrieme liste de 24 à 31
    _noyau_tcb_add[id].Nb_tour = 1;
    _noyau_tcb_add[id].wait_time = 1000;
    active(cree(tachePeriodique, id, (void *)&_noyau_tcb_add[id]));

// *************************
// TACHES APERIODIQUES
// *************************

    //ACHETER DES OUTILS




	start();
  return(0);
}

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