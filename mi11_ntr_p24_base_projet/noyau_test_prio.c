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

TACHE	tachedefond(void);
TACHE	tacheGen(void);

#define MAX_CARA_LIGNE 80

/*
 * structure du contexte d'une tache
 */
typedef struct {
    // adresse de debut de la tache
    uint16_t Nb_tour;
    // etat courant de la tache
    uint16_t wait_time;
} NOYAU_TCB_ADD;

/*----------------------------------------------------------------------------*
 * declaration des variables du noyau comme extern pour pouvoir les
 * utiliser dans d'autres partie du code
 * *--------------------------------------------------------------------------*/
#define POS_CHRONO 10
/*
 * tableau stockant le contexte de chaque tache
 */
NOYAU_TCB_ADD _noyau_tcb_add[MAX_TACHES_NOYAU];

uint16_t pos_x = 1;
uint16_t pos_y = 10;


TACHE	tachedefond(void)
{
	uint16_t id;

	SET_CURSOR_POSITION(3,1);
	puts("------> EXEC tache de fond");

	id = 3;
		_noyau_tcb_add[id].Nb_tour = 1;
		_noyau_tcb_add[id].wait_time = 100;
		active(cree(tacheGen, id, (void*)&_noyau_tcb_add[id]));
	id = 8;
		_noyau_tcb_add[id].Nb_tour = 2;
		_noyau_tcb_add[id].wait_time = 50;
		active(cree(tacheGen, id, (void*)&_noyau_tcb_add[id]));
 	id = 16;
		_noyau_tcb_add[id].Nb_tour = 4;
		_noyau_tcb_add[id].wait_time = 60;
		active(cree(tacheGen, id, (void*)&_noyau_tcb_add[id]));
	id = 18;
		_noyau_tcb_add[id].Nb_tour = 4;
		_noyau_tcb_add[id].wait_time = 40;
		active(cree(tacheGen, id, (void*)&_noyau_tcb_add[id]));
  	id = 24;
		_noyau_tcb_add[id].Nb_tour = 3;
		_noyau_tcb_add[id].wait_time = 15;
		active(cree(tacheGen, id, (void*)&_noyau_tcb_add[id]));
	id = 31;
		_noyau_tcb_add[id].Nb_tour = 3;
		_noyau_tcb_add[id].wait_time = 15;
		active(cree(tacheGen, id, (void*)&_noyau_tcb_add[id]));
	id = 32;
		_noyau_tcb_add[id].Nb_tour = 2;
		_noyau_tcb_add[id].wait_time = 10;
		active(cree(tacheGen, id, (void*)&_noyau_tcb_add[id]));
	id = 37;
		_noyau_tcb_add[id].Nb_tour = 3;
		_noyau_tcb_add[id].wait_time = 10;
		active(cree(tacheGen, id, (void*)&_noyau_tcb_add[id]));
	id = 40;
		_noyau_tcb_add[id].Nb_tour = 3;
		_noyau_tcb_add[id].wait_time = 5;
		active(cree(tacheGen, id, (void*)&_noyau_tcb_add[id]));
	id = 48;
		_noyau_tcb_add[id].Nb_tour = 5;
		_noyau_tcb_add[id].wait_time = 2;
		active(cree(tacheGen, id, (void*)&_noyau_tcb_add[id]));
	id = 56;
		_noyau_tcb_add[id].Nb_tour = 5;
		_noyau_tcb_add[id].wait_time = 1;
		active(cree(tacheGen, id, (void*)&_noyau_tcb_add[id]));

	while(1) {};

}

TACHE	tacheGen(void)
{
	volatile NOYAU_TCB* p_tcb = NULL;
	volatile uint16_t id_tache;
	uint16_t i, j=1;


	id_tache = noyau_get_tc();
	p_tcb = noyau_get_p_tcb(id_tache);

	volatile uint16_t Nb_tour = ((NOYAU_TCB_ADD*)(p_tcb->tcb_add))->Nb_tour;
	volatile uint16_t wait_time = ((NOYAU_TCB_ADD*)(p_tcb->tcb_add))->wait_time;


	// on laisse du temps à la tâche de fond de démarrer toutes les tâches
	delay_n_ticks(20);
	while(1){
		//id_tache = noyau_get_tc();
		while(tache_get_flag_tick(id_tache) != 0){
			_lock_();
			for(i = POS_CHRONO; i<(POS_CHRONO+8); i++){
				printf("%s%d;%d%s",CODE_ESCAPE_BASE,i, pos_x,"H");
				if((i-POS_CHRONO) == (id_tache >> 3) ){
					SET_CURSOR_POSITION( i, pos_x);
					SET_BACKGROUND_COLOR(id_tache+16);
					SET_FONT_COLOR(15);
					printf("%2d", id_tache);
					SET_BACKGROUND_COLOR(0);
				} else {
					SET_BACKGROUND_COLOR(0);
					printf("  ");
				}
			}
			pos_x = pos_x + 2;
			if (pos_x > MAX_CARA_LIGNE){
				pos_x = 1;
			}

			if (j >= Nb_tour){
					j = 1;
					delay_n_ticks(wait_time);
			} else {
					j++;
			}
			_unlock_();
			tache_reset_flag_tick(id_tache);
		}
	}
}



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
	start(tachedefond);
  return(0);
}
