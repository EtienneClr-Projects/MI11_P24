/* NOYAU_PRIO.C */
/*--------------------------------------------------------------------------*
 *               Code C du noyau preemptif qui tourne sur ARM                *
 *                                 NOYAU.C                                  *
 *--------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>

#include "stm32h7xx.h"
#include "serialio.h"
#include "noyau_prio.h"
#include "noyau_file_prio.h"
#include "delay.h"
#include "TERMINAL.h"

/*--------------------------------------------------------------------------*
 *           Constantes pour la création du contexte CPU initial            *
 *--------------------------------------------------------------------------*/
#define THUMB_ADDRESS_MASK (0xfffffffe) /* Masque pointeurs de code Thumb   */
#define TASK_EXC_RETURN (0xfffffff9)    /* Valeur de retour d'exception :   */
                                        /* mspnamed active, mode Thread          */
#define TASK_PSR (0x01000000)           /* PSR initial                      */

/*--------------------------------------------------------------------------*
 *            Variables internes du noyau                                   *
 *--------------------------------------------------------------------------*/
static int compteurs[MAX_TACHES_NOYAU+1];                      /* Compteurs d'activations               */
NOYAU_TCB _noyau_tcb[MAX_TACHES_NOYAU];                      /* tableau des contextes                 */
NOYAU_TCB_APERIODIC _noyau_tcb_apperiodic[MAX_TACHES_NOYAU]; /* tableau des contextes pour les tâches apériodiques               */
uint16_t _ap_cmpt = 0;
volatile uint16_t _tache_c; /* numéro de tache courante              */
uint32_t _tos;              /* adresse du sommet de pile des tâches  */
uint8_t _ack_timer = 1;     /* variable de détection d'appel SYSTICK */

/*----------------------------------------------------------------------------*
 * fonctions du noyau                                                         *
 *----------------------------------------------------------------------------*/

/*
 * termine l'execution du noyau
 * entre  : sans
 * sortie : sans
 * description : termine l'execution du noyau, execute en boucle une
 *               instruction vide
 */
void noyau_exit(void)
{
    uint16_t j;
    /* Q2.1 : desactivation des interruptions */
    _irq_disable_();

    /* affichage du nombre d'activation de chaque tache !*/
    printf("Sortie du noyau\n");
    for (j = 0; j < MAX_TACHES_NOYAU; j++)
    {
        printf("\nActivations tache %d : %d", j, compteurs[j]);
    }
    /* Q2.2 : Que faire quand on termine l'execution du noyau ? */
    for (;;)
        continue; /* Terminer l'exécution                     */
}

/*
 * termine l'execution d'une tache
 * entre  : sans
 * sortie : sans
 * description : une tache dont l'execution se termine n'est plus executee
 *               par le noyau
 *               cette fonction doit etre appelee a la fin de chaque tache
 */
void fin_tache(void)
{
    /* Q2.3 :             Début section critique                             */
    _lock_();
    /* Q2.4 : la tache est enlevee de la file des taches et ... ?  */
    /* 4 instructions */
    _noyau_tcb[_tache_c].status = CREE;
    file_retire(_tache_c); /* la tache est enlevee de la file          */
    scheduler();           /* Activation d'une tâche prête             */
    _unlock_();            /* Fin section critique                     */
}

/*
 * demarrage du system en mode multitache
 * entre  : adresse de la tache a lancer
 * sortie : sans
 * description : active la tache et lance le scheduler
 */
void start(void)
{
    uint16_t j;
    register unsigned int sp asm("sp");

    /* Q2.5 : initialisation de l'etat des taches                            */
    for (j = 0; j < MAX_TACHES_NOYAU; j++)
    {
        _noyau_tcb[j].status = NCREE;            /* initialisation de l'etat des taches */
        _noyau_tcb_apperiodic[j].status = NCREE; /* initialisation de l'etat des taches apériodiques */
    }

    /* Q2.6 : initialisation de la tache courante */
    _tache_c = 63; // tache de fond
    /* initialisation de la file circulaire de gestion des tâches           */
    file_init();
    /* Q2.7 : initialisation de la variable _tos sommet de la pile           */
    /* Haut de la pile des tâches avec réservation  pour le noyau  */
    _tos = sp - PILE_NOYAU; /* Haut de la pile des tâches          */

    /* Q2.8 : on interdit les interruptions  */
    _irq_disable_();
    /* Q2.9 : initialisation du timer system a 100 Hz   (voir cortex.c)    */
    systick_start(CORE_CLK / 100);
    /* Q2.10 : initialisation de l'interruption systick  (voir cortex.c) */
    systick_irq_enable();
    /* Q2.11 : creation et activation de la premiere tache                          */
    active(cree(tachedefond, 63, NULL));


    /* Q2.12 : on autorise les interruptions */
    _irq_enable_();
}

/*
 * creation d'une nouvelle tache
 * entre  : adresse de la tache a creer
 * sortie : numero de la tache cree
 * description : la tache est creee en lui allouant une pile et un numero
 *               en cas d'erreur, le noyau doit etre arrete
 * Err. fatale: priorite erronnee, depassement du nb. maximal de taches
 */
uint16_t cree(TACHE_ADR adr_tache, uint16_t id, void *add)
{
    /* pointeur d'une case de _noyau_tcb         */
    NOYAU_TCB *p;

    /* Q2.14: debut section critique */
    _lock_();

    /* Q2.16 : arret du noyau si plus de MAX_TACHES^*/
    if (id >= MAX_TACHES_NOYAU)
    {
        noyau_exit();
    }
    if (_noyau_tcb[id].status != NCREE)
    {
        noyau_exit();
    }
    /* creation du contexte de la nouvelle tache */
    p = &_noyau_tcb[id];
    /* Q2.17 : allocation d'une pile a la tache */
    p->sp_ini = _tos;
    /* Q2.18 : decrementation du pointeur de pile general, afin que la prochaine tache */
    /* n'utilise pas la pile allouee pour la tache courante */
    _tos -= PILE_TACHE;
    /* Q2.19 : memorisation de l'adresse de debut de la tache */
    p->task_adr = adr_tache;
    p->prio = id >> 3;
    p->id = id;
    p->tcb_add = add;
    /* initialisation du compteur de délai à zéro */
    p->cmpt = 0;
    /* Q2.20 : mise a jour de l'etat de la tache a CREE */
    p->status = CREE;
    /* Q2.21 : fin section critique */
    _unlock_();
    printf("%d", id);
    return (id); /* tache est un uint16_t */
}

uint16_t cree_aperiodic(TACHE_ADR adr_tache, void *add)
{
    NOYAU_TCB_APERIODIC *p;

    _lock_();
    if (_ap_cmpt >= MAX_TACHES_NOYAU)
    {
        // il faut changer la ligne en dessous car on ne veut plus de noyau exit
        noyau_exit();
    }
    if (_noyau_tcb_apperiodic[_ap_cmpt].status != NCREE)
    {
        // il faut changer la ligne en dessous car on ne veut plus de noyau exit
        noyau_exit();
    }

    /* creation du contexte de la nouvelle tache */
    /* Q2.17 : allocation d'une pile a la tache */
    p->sp_ini = _tos;
    /* Q2.18 : decrementation du pointeur de pile general, afin que la prochaine tache */
    /* n'utilise pas la pile allouee pour la tache courante */
    _tos -= PILE_TACHE;
    /* Q2.19 : memorisation de l'adresse de debut de la tache */
    p->task_adr = adr_tache;
    p->tcb_add = add;
    p->status = CREE;
    _unlock_();

    return _ap_cmpt++;
}

/*
 * ajout d'une tache pouvant etre executee a la liste des taches eligibles
 * entre  : numero de la tache a ajouter
 * sortie : sans
 * description : ajouter la tache dans la file d'attente des taches eligibles
 *               en cas d'erreur, le noyau doit etre arrete
 */
void active(uint16_t tache)
{
    NOYAU_TCB *p = &_noyau_tcb[tache]; /* acces au contexte tache             */

    /* Q2.22 : verifie que la tache n'est pas dans l'etat NCREE, sinon arrete le noyau*/
    if (p->status == NCREE)
    {
        noyau_exit(); /* sortie du noyau                      */
    }

    /* Q2.23 : debut section critique */
    _lock_();

    /* Q2.24 : activation de la tache seulement si elle est a l'état CREE    */
    if (p->status == CREE)
    { /* n'active que si receptif  */
        /* Créer un contexte initial en haut de la pile de la nouvelle tâche */
        p->sp_start = p->sp_ini - sizeof(CONTEXTE_CPU_BASE); /* Réserver la Place pour
                                                                le contexte sur la pile de la tâche */
        p->sp_start &= 0xfffffff8;                           /* Aligner au multiple de 8 inférieur   */
        CONTEXTE_CPU_BASE *c = (CONTEXTE_CPU_BASE *)p->sp_start;
        c->pc = ((uint32_t)p->task_adr) & THUMB_ADDRESS_MASK; /* Adresse de la tâche dans pc, attention au bit de poids faible*/
        uint32_t ddd = ((uint32_t)p->task_adr) & THUMB_ADDRESS_MASK;
        printf("%d", ddd);
        c->lr = ((uint32_t)p->task_adr);                      /* Adresse de retour de la tâche dans lr */
        c->lr_exc = TASK_EXC_RETURN;                          /* veleur initiale de retour d'exeption dans lr_exc */
        c->psr = TASK_PSR;                                    /* veleur initiale des flags dans psr    */

        p->status = PRET;   /* changement d'etat, mise a l'etat PRET */

        file_ajoute(tache); /* ajouter la tache dans la liste        */
        scheduler();        /* activation d'une tache prete          */
    }
    /* Q2.25 : fin section critique */
    _unlock_();
}

// void active_aperiodic(uint16_t tache)
// {
//     NOYAU_TCB_APERIODIC *p = &_noyau_tcb[tache]; /* acces au contexte tache             */

//     /* Q2.22 : verifie que la tache n'est pas dans l'etat NCREE, sinon arrete le noyau*/
//     if (p->status == NCREE)
//     {
//         noyau_exit(); /* sortie du noyau                      */
//     }

//     /* Q2.23 : debut section critique */
//     _lock_();

//     /* Q2.24 : activation de la tache seulement si elle est a l'état CREE    */
//     if (p->status == CREE)
//     { /* n'active que si receptif  */
//         /* Créer un contexte initial en haut de la pile de la nouvelle tâche */
//         p->sp_start = p->sp_ini - sizeof(CONTEXTE_CPU_BASE); /* Réserver la Place pour
//                                                                 le contexte sur la pile de la tâche */
//         p->sp_start &= 0xfffffff8;                           /* Aligner au multiple de 8 inférieur   */
//         CONTEXTE_CPU_BASE *c = (CONTEXTE_CPU_BASE *)p->sp_start;
//         c->pc = ((uint32_t)p->task_adr) & THUMB_ADDRESS_MASK; /* Adresse de la tâche dans pc, attention au bit de poids faible*/
//         c->lr = ((uint32_t)p->task_adr);                      /* Adresse de retour de la tâche dans lr */
//         c->lr_exc = TASK_EXC_RETURN;                          /* veleur initiale de retour d'exeption dans lr_exc */
//         c->psr = TASK_PSR;                                    /* veleur initiale des flags dans psr    */

//         p->status = PRET;   /* changement d'etat, mise a l'etat PRET */
//         file_ajoute(tache); /* ajouter la tache dans la liste        */
//         scheduler();        /* activation d'une tache prete          */
//     }
//     /* Q2.25 : fin section critique */
//     _unlock_();
// }

/*--------------------------------------------------------------------------*
 *                  ORDONNANCEUR preemptif optimise                         *
 * Entrée : pointeur de contexte CPU de la tâche courante                   *
 * Sortie : pointeur de contexte CPU de la nouvelle tâche courante          *
 * Descrip: sauvegarde le pointeur de contexte de la tâche courante,        *
 *      Élit une nouvelle tâche et retourne son pointeur de contexte.       *
 *      Si aucune tâche n'est éligible, termine l'exécution du noyau.       *
 *--------------------------------------------------------------------------*/
uint32_t task_switch(uint32_t sp)
{
    NOYAU_TCB *p = &_noyau_tcb[_tache_c]; /* acces au contexte tache courante */

    /* Q2.26 : sauvegarde du pointeur sur le contexte sauvegardé sur la pile
       dans le contexte de la tache */
    p->sp = sp;

    if (_ack_timer)
    {
        delay_process();
        flag_tick_process();
    }
    else
    {
        _ack_timer = 1;
    }

    /* on bascule sur la nouvelle tache a executer */
    /* Q2.27 : recherche la prochaine tache a executer */
    _tache_c = file_suivant();
    /* Q2.28 : acces contexte suivant                   */
    p = &_noyau_tcb[_tache_c];
    /* Q2.29 : verifie qu'une tache suivante existe, sinon arret du noyau */
    if (_tache_c == F_VIDE)
    {
        printf("Plus rien à ordonnancer.\n");

        _tache_c = 63;

    }

    compteurs[_tache_c]++; /* MAJ compteur d'activations               */
    /* Q2.30 : retourner la bonne valeur de pointeur de pile
     Deux cas possible en fonction du statut de la tâche */
    if (p->status == PRET)
    {
        p->status = EXEC;
        return p->sp_start;
    }
    else
    {
        return p->sp;
    }
}

/*--------------------------------------------------------------------------*
 *              --- Gestionnaire d'exception PEND SVC ---                   *
 * Descrip: Appelé lors de l'exception PEND SVC provoquée par pendsv_trigger*
 *      Sauvegarde le contexte CPU sur la pile de la tâche et provoque une  *
 *      commutation de contexte.                                            *
 *------------------------------------------------------------------------- */
void __attribute__((naked)) _pend_svc(void)
{
    /* Q2.31 : sauvegarde du complément de conyexte et appel de
                 l'ordonnaceur */
    __asm__ __volatile__(
        "push   {r4-r11,lr}\n" /* Sauvegarder le complément de  contexte
                                  sur la pile                          */
        "mov    r0, sp     \n" /* r0 = 1er paramètre de task_switch    */
        "bl     task_switch\n" /* Commutation de contexte              */
        "mov    sp, r0     \n" /* r0 = valeur de retour de task_switch
                                  dans ? */
        "pop    {r4-r11,lr}\n" /* Restituer le contexte                */
        "bx     lr         \n" /* Retour d'exception                   */
    );
}

/*--------------------------------------------------------------------------*
 *                  --- Provoquer une commutation ---                       *
 *                                                                          *
 * Descrip: planifie une exception PEND SVC pour forcer une commutation     *
 *      de tâche                                                            *
 *--------------------------------------------------------------------------*/
void schedule(void)
{
    pendsv_trigger();
}

void scheduler(void)
{
    _lock_();
    _ack_timer = 0;
    schedule();
    _unlock_();
}

/*-------------------------------------------------------------------------*
 *                    --- Endormir la tâche courante ---                   *
 * Entree : Neant                                                          *
 * Sortie : Neant                                                          *
 * Descrip: Endort la tâche courante et attribue le processeur à la tâche  *
 *          suivante.                                                      *
 *                                                                         *
 * Err. fatale:Neant                                                       *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void dort(void)
{
    _lock_();
    _noyau_tcb[_tache_c].status = SUSP;
    file_retire(_tache_c);
    scheduler();
    _unlock_();
}

/*-------------------------------------------------------------------------*
 *                    --- Réveille une tâche ---                           *
 * Entree : numéro de la tâche à réveiller                                 *
 * Sortie : Neant                                                          *
 * Descrip: Réveille une tâche. Les signaux de réveil ne sont pas mémorisés*
 *                                                                         *
 * Err. fatale:tâche non créée                                             *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void reveille(uint16_t t)
{
    NOYAU_TCB *p;

    p = &_noyau_tcb[t];

    if (p->status == NCREE)
        noyau_exit();

    _lock_();
    if (p->status == SUSP)
    {
        p->status = EXEC;
        file_ajoute(t);
    }
    scheduler();
    _unlock_();
}

/*
 * recupere le numero de tache courante
 * entre  : sans
 * sortie : valeur de _tache_c
 * description : fonction d'acces a la valeur d'identite de la tache courante
 */
uint16_t noyau_get_tc(void)
{
    return (_tache_c);
}

NOYAU_TCB *noyau_get_p_tcb(uint16_t tcb_nb)
{
    return &_noyau_tcb[tcb_nb];
}

uint8_t tache_get_flag_tick(uint16_t id_tache)
{
    return (_noyau_tcb[id_tache].flag_tick);
}

void tache_reset_flag_tick(uint16_t id_tache)
{
    _noyau_tcb[id_tache].flag_tick = 0;
}

void tache_set_flag_tick(uint16_t id_tache)
{
    _noyau_tcb[id_tache].flag_tick = 1;
}

void flag_tick_process(void)
{
    uint16_t id_tache;

    for (id_tache = 0; id_tache < MAX_TACHES_NOYAU; id_tache++)
    {
        tache_set_flag_tick(id_tache);
    }
}



TACHE tachedefond(void) // ordonnanceur des taches aperiodiques
{
    printf("tache de fond\n");

    // ici on execute les taches aperiodiques
    uint8_t index_to_execute;
    if (fifo_retire(&fifo_tache_aperiodic, (uint8_t*)&index_to_execute) == -1)
    {
        // exec
        ((TACHE_ADR)tab_tache_aperiodic[index_to_execute].adr)(tab_tache_aperiodic[index_to_execute].params);
//        ((TACHE_ADR)tab_tache_aperiodic[index_to_execute].adr)(tab_tache_aperiodic[index_to_execute].params);
    }
    else 
    {
        printf("fifo empty, NOYAU EXIT\n");
        noyau_exit();
    }   
}

