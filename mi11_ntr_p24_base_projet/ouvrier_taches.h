#include <stdio.h>
#include <stdlib.h>
#include <time.h>


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

