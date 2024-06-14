- quand on a plus de taches periodiques, on fait pas un noyau exit, on met tache_c à 64 ou 63???
- on cree une TACHE tachedefond avec un id 64
- faire un creerTacheAP(TACHE_ADR adr, void* params) c'est comme cree
void creerTacheAperiodique(TACHE_ADR adr, int16_t id, char* name, void *params){
	_lock_();
	if(ap_task_count < MAX_AP_TASKS){
		ap_task_list[ap_task_count].adr_tache = adr;
		ap_task_list[ap_task_count].params = params;
		ap_task_list[ap_task_count].active = 1;
		ap_task_count++;

	}else{
		printf("Plus de place pour une nouvelle tâche dans la file.\n");
	}
	schedule();
	_unlock_();
}

- définir une structure TACHE_AP qui contient une TACHE_ADR, des params et un flag actif
typedef struct {
	TACHE_ADR adr_tache;
	void *params;
	uint8_t active;
} AP_TASK;

- définir un tableau de TACHE_AP ap_task_list[MAX_AP_TASKS] et un compteur ap_task_count

- une fonction tacheAP comme tacheGEN, mais pour les taches aperiodiques avec un arg(void*), dedans on fait un print, et on delay_n_ticks

- dans tachedefond, on a la file des fonctions AP à executer par ordre de prio
et on les parcourt et si elles sont actives, on les execute et on desactive
tache_reset_flag_tick(id_tache) avec id_tache la noyau_get_tcb() ou 64 nan??
