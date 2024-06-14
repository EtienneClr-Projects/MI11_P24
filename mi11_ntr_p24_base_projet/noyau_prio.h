/* NOYAU.H */
/*--------------------------------------------------------------------------*
 *  mini noyau temps reel                                                   *
 *		    NOYAU.H                                                 *
 ****************************************************************************
 * On definit dans ce fichier toutes les constantes et les structures       *
 * necessaires au fonctionnement du noyau
 *--------------------------------------------------------------------------*/

#ifndef __NOYAU_H__
#define __NOYAU_H__

#include <stdint.h>

/* Les constantes */
/******************/

#define PILE_TACHE 2048 /* Taille maxi de la pile d'une tâche   */
#define PILE_NOYAU 512  /* Place réservée pour la pile noyau    */

/*  Definitions des fonctions dependant du materiel sous forme
 *  de macros.
 ***************************************************************/

#define _irq_enable_() __asm__ __volatile__( \
    "cpsie i            \n"                  \
    "dsb                \n"                  \
    "isb                \n")

#define _irq_disable_() __asm__ __volatile__( \
    "cpsid i            \n"                   \
    "dsb                \n"                   \
    "isb                \n")

#define _lock_() __asm__ __volatile__( \
    "mrs    r0, PRIMASK \n"            \
    "push   {r0}        \n"            \
    "cpsid  i           \n"            \
    "dsb                \n"            \
    "isb                \n" ::: "r0")

#define _unlock_() __asm__ __volatile__( \
    "pop    {r0}        \n"              \
    "msr    PRIMASK, r0 \n"              \
    "dsb                \n"              \
    "isb                \n" ::: "r0")

/* Contexte CPU complet d'une tâche */
/************************************/
typedef struct __attribute__((packed, aligned(4)))
{
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t lr_exc;
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t pc;
  uint32_t psr;
} CONTEXTE_CPU_BASE;

/* Etat des taches */
/*******************/

#define NCREE 0     /* Etat non cree          */
#define CREE 0x8000 /* Etat cree ou dormant   */
#define PRET 0x9000 /* Etat eligible          */
#define SUSP 0xA000 /* Etat suspendu          */
#define EXEC 0xC000 /* Etat execution         */

/* Definition des types */
/************************/

#define TACHE void
typedef TACHE (*TACHE_ADR)(void); /* pointeur de taches      */

/* definition du contexte d'une tache */
/**************************************/

typedef struct
{
  uint16_t status;    /* etat courant de la tache        */
  uint32_t sp_ini;    /* valeur initiale de sp           */ // SP c'est le stack pointer, donc l'adresse de la pile
  uint32_t sp_start;  /* valeur de base de sp pour la tache */ 
  uint32_t sp;        /* valeur courante de sp           */
  TACHE_ADR task_adr; /* Pointeur de la fonction de tâche*/
  uint16_t prio;      /* priorité de la tâche */
  uint16_t id;        /* identité de la tâche */
  uint32_t cmpt;      /* valeur courante decomptage pour reveil */
  uint8_t flag_tick;  /* indicateur permettant à la tâche de savoir
                           qu'un tick horloge est passé depuis une période précédente */
  void *tcb_add;      /* pointeur sur des paramètres supplémentaires pour la tâches */
} NOYAU_TCB;

typedef struct
{
  uint16_t status;    /* etat courant de la tache        */
  uint32_t sp_ini;    /* valeur initiale de sp           */
  uint32_t sp_start;  /* valeur de base de sp pour la tache */
  uint32_t sp;        /* valeur courante de sp           */
  TACHE_ADR task_adr; /* Pointeur de la fonction de tâche*/
  uint16_t id;        /* identité de la tâche */
  void *tcb_add;      /* pointeur sur des paramètres supplémentaires pour la tâches */
} NOYAU_TCB_APERIODIC;

/* Prototype des fonctions */
/***************************/

void noyau_exit(void);
void fin_tache(void);
uint16_t cree(TACHE_ADR adr_tache, uint16_t id, void *add);
uint16_t cree_aperiodic(TACHE_ADR adr_tache, void *add);
void active(uint16_t tache);
void active_aperiodic(uint16_t tache);
void schedule(void);
void scheduler(void);
void start(void);
void dort(void);
void reveille(uint16_t tache);
uint16_t noyau_get_tc(void);
NOYAU_TCB *noyau_get_p_tcb(uint16_t tcb_nb);
uint8_t tache_get_flag_tick(uint16_t id_tache);
void tache_reset_flag_tick(uint16_t id_tache);
void tache_set_flag_tick(uint16_t id_tache);
void flag_tick_process(void);

#endif
