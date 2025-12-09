/**
* @file sokoban.c
* @brief Programme qui fait tourner un jeu de sokoban
* @author Guillaume ANTOINES
* @version 2.0
* @date 30/11/2025
*
* Ce programme fait tourner un jeu de sokoban dont le but est de déplacer
* toutes les caisses sur des cibles pour gagner la partie.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

// Définition de la taille du tableau.
#define MAXLIG 12
#define MAXDEP 10000
#define TAILLE_FICHIER 50

typedef char t_plateau[MAXLIG][MAXLIG];
typedef char typeDeplacements[MAXDEP];

//Définition de la structure de jeu
typedef struct{
	int posx; // position horizontale du joueur
	int posy; // position verticale du joueur
	int nbDep; // nombre de déplacements effectués
	int animation; // nombre de jeu->animations sur l'entête
	t_plateau plateau; // déclaration du plateau de jeu
	typeDeplacements historiqueDep; // déclaration du tableau des déplacements
} t_partie;


// Définition des caractères constantes.
const char CAISSE = '$';
const char MUR = '#';
const char JOUEUR = '@';
const char CIBLE = '.';
const char JOUEUR_CIBLE = '+';
const char CAISSE_CIBLE = '*';
const char CASE = ' ';

// Définition des caractères de déplacement
const char DEP_GAUCHE = 'g';
const char DEP_DROITE = 'd';
const char DEP_HAUT = 'h';
const char DEP_BAS = 'b';
const char RETOUR = 'u';
const char CAISSE_GAUCHE = 'G';
const char CAISSE_DROITE = 'D';
const char CAISSE_HAUT = 'H';
const char CAISSE_BAS = 'B';


// liste des procédures déclarées
void chargerPartie(t_plateau plateau, char fichier[]);
void chargerDeplacements(typeDeplacements t, char fichier[], int * nb);
void afficher_entete(t_partie *jeu, char fichier[], char deplacements[]);
void afficher_plateau(t_partie *jeu);
void chercher_joueur(t_partie *jeu);
void conditions_dep(t_partie *jeu, int depx, int depy, char touche);
void deplacer_joueur(t_partie *jeu, int depx, int depy);
void deplacer_caisse(t_partie *jeu, int depx, int depy, int casx, int casy);
void annuler_deplacer(t_partie *jeu, char last);
void Analyse(t_partie *jeu, char fichier[], char deplacements[]);
bool gagner(t_partie *jeu);

/**
* @brief coeur du programme
* Initialise la partie, charge le niveau et lance le jeu.
* @return EXIT_SUCCESS: arrêt normal du programme
*/

int main(){
	t_partie jeu;
	jeu.posx = 0; // initialisation de la position
	jeu.posy = 0; 
	jeu.nbDep = 0; // initialisation du nombre de déplacements
	jeu.animation = 1;
	int maxTaille; // nombre de caractères dans le tableau des déplacements
	char fichier[TAILLE_FICHIER]; // le nom du fichier de la partie
	char deplacements[TAILLE_FICHIER]; // le nom du fichier des déplacements

	// sélection du niveau
	printf("Quel niveau voulez vous charger ? (ex: niveau1.sok) : ");
	scanf("%s", fichier); // sélection du fichier de la partie
	chargerPartie(jeu.plateau, fichier); // charge le fichier du plateau
	
	printf("Entrez le nom du fichier de déplacements (ex: niveau1.sok) : ");
	scanf("%s", deplacements); // sélection du fichier des déplacements
	chargerDeplacements(jeu.historiqueDep, deplacements, &maxTaille);

	system("clear");
	afficher_entete(&jeu, fichier, deplacements); 
	afficher_plateau(&jeu);
	chercher_joueur(&jeu);
	// tant qu'il y a des caisses à déplacer
	while (jeu.nbDep < maxTaille && !gagner(&jeu)) {
		usleep(250000); // pause de 0.25 seconde
		Analyse(&jeu, fichier, deplacements);
		jeu.nbDep++;
	}

	// affichage des résultats
	if (gagner(&jeu)) {
		printf("La suite de déplacements %s est bien une solution pour la partie %s.\n", fichier, deplacements);
		if (jeu.nbDep < maxTaille){
			printf("Elle contenait de base %d caractères\n", maxTaille);
		}
		printf("La partie contient actuellement %d déplacements.\n", jeu.nbDep);
	} 
	
	else {
		printf("La suite de déplacements %s N'EST PAS une solution pour la partie %s! \n", fichier, deplacements);
	}
	return EXIT_SUCCESS;
}

/**
* @brief charge les caractères sur lignes et colonnes de la partie
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @param fichier type : entier, entrée, fichier de la partie chargée
* @return résultat : chargement de la partie
*/

void chargerPartie(t_plateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne;
	int TAILLE = 12;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne=0 ; ligne<TAILLE ; ligne++){
            for (int colonne=0 ; colonne<TAILLE ; colonne++){
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

/**
* @brief charge les caractères sur lignes et colonnes de la partie
* @param t type : tableau, entrée/sortie, importe le tableau des déplacements
* @param fichier type : chaine, entrée, fichier des déplacements 
* @param nb type : entier, entrée/sortie, nombre de caractères chargés
* @return résultat : chargement du fichier des caractères
*/

void chargerDeplacements(typeDeplacements t, char fichier[], int * nb){
    FILE * f;
    char dep;
    *nb = 0;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("FICHIER NON TROUVE\n");
    } else {
        fread(&dep, sizeof(char), 1, f);
        if (feof(f)){
            printf("FICHIER VIDE\n");
        } else {
            while (!feof(f)){
                t[*nb] = dep;
                (*nb)++;
                fread(&dep, sizeof(char), 1, f);
            }
        }
    }
    fclose(f);
}

/**
* @brief enregistre les caractères de chaque ligne et colonne de la partie
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @param fichier type : entier, entrée, fichier de la partie chargée
* @return résultat : affichage du plateau
*/

void enregistrerPartie(t_plateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne='\n';
	int TAILLE = 12;


    f = fopen(fichier, "w");
    for (int ligne=0 ; ligne<TAILLE ; ligne++){
        for (int colonne=0 ; colonne<TAILLE ; colonne++){
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}

/**
* @brief affiche les information du jeu
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @param fichier type : entier, entrée, fichier de la partie chargée
* @param deplacements type : entier, entrée, fichier des déplacements
* @param animation : entier, entrée, affiche une animation de 1 à 3 points
* @return résultat : affichage de l'entête du plateau
*/

void afficher_entete(t_partie *jeu, char fichier[], char deplacements[]){

    // Gestion du cycle
    if (jeu->animation > 3) {
        jeu->animation = 1;
    }

	system("clear");
	printf(" Nom de la partie : %s\n\n", fichier);
	printf(" Nom du fichier de déplacements : %s\n\n", deplacements);
	printf(" Nombre de déplacement : %d\n\n", jeu->nbDep);

	if (jeu->animation == 1){
		printf(" Analyse en cours.\n\n");
	} 
	else if (jeu->animation == 2){
		printf(" Analyse en cours..\n\n");
	}
	else if (jeu->animation == 3){
		printf(" Analyse en cours...\n\n");
	}
	jeu->animation++;
}

/**
* @brief affiche le plateau de jeu
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @return résultat : affiche du nombre de caractères sur le tableau
*/

void afficher_plateau(t_partie *jeu) {
	char caractere;
	for (int lig=0; lig < MAXLIG; lig++) {
		for (int col=0; col < MAXLIG; col++) {
			caractere = jeu->plateau[lig][col];
			// pour afficher correctement le joueur et la caisse sur cible 
			if (caractere == JOUEUR_CIBLE) {
				printf("%c", JOUEUR);
			}
			else if (caractere == CAISSE_CIBLE) {
				printf("%c", CAISSE);
			}
			else{
				printf("%c", caractere);
			}
		}
		printf("\n");
	}
}

/**
* @brief cherche le caractère correspondant du joueur (@)
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @param posx type : entier, entrée/sortie, position horizontale joueur
* @param posy type : entier, entrée/sortie, position verticale joueur
* @return résultat : joueur trouvé si présent
*/

void chercher_joueur(t_partie *jeu){
	for (int lig=0; lig < MAXLIG; lig++) {
		for (int col=0; col < MAXLIG; col++) {
			// si on trouve le joueur
			if ((jeu->plateau[lig][col] == JOUEUR) ||
				 (jeu->plateau[lig][col] == JOUEUR_CIBLE)) {
				jeu->posx = lig; // position horizontale trouvé
				jeu->posy = col; // position verticale trouvé
			}
		}
	}
}

/**
* @brief introduit les conditions de déplacement de la caisse
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @param historiqueDep type : tableau, entrée/sortie, importe le tableau des
	déplacements.
* @param posy type : entier, entrée, position verticale du joueur
* @param posx type : entier, entrée, position horizontale du joueur
* @param depx type : entier, entrée, case de destination horizontale ou
	position de la caisse
* @param depy type : entier, entrée, case de destination verticale
	ou position de la caisse
* @param nbDep type : entier, entrée/sortie, nombre de déplacements effectués
* @param last type : caractère, entrée, dernier caractère de déplacement
* @return résultat : déplacement ou non dans le plateau
*/

void conditions_dep(t_partie *jeu, int depx, int depy, char last){
	
	int casx; // case de destination de la caisse
	int casy; 

	if (jeu->plateau[depx][depy] != MUR) {

		if (last == CAISSE_HAUT ||
			last == CAISSE_BAS ||
			last == CAISSE_GAUCHE ||
			last == CAISSE_DROITE) {

			// calcul de la case de destination de la caisse
			casx = depx + (depx - jeu->posx);
			casy = depy + (depy - jeu->posy);
			// colision avec un mur ou une autre caisse
			if ((jeu->plateau[casx][casy] != MUR) && 
				(jeu->plateau[casx][casy] != CAISSE) && 
				(jeu->plateau[casx][casy] != CAISSE_CIBLE)) {
				deplacer_caisse(jeu, depx, depy, casx, casy);
				deplacer_joueur(jeu, depx, depy);
			}
		}
		// Uniquement les déplacements du joueur
		else {
			deplacer_joueur(jeu, depx, depy);
		}
	}
}

/**
* @brief introduit les conditions de déplacement du joueur
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @param depx type : entier, entrée, case de déplacement horizontale joueur
* @param depy type : entier, entrée, case de déplacement verticale joueur
* @param casx type : entier, entrée/sortie, case de déplacement horizontale caisse
* @param casy type : entier, entrée/sortie, case de déplacement verticale caisse
* @return résultat : le joueur peut/peut pas se déplacer
*/

void deplacer_joueur(t_partie *jeu, int depx, int depy){
	// si le joueur est déplacé depuis une cible
	if (jeu->plateau[jeu->posx][jeu->posy] == JOUEUR_CIBLE) {
		jeu->plateau[jeu->posx][jeu->posy] = CIBLE;
	}
	else {
		jeu->plateau[jeu->posx][jeu->posy] = CASE;
	}
	jeu->posx = depx; // mise à jour de la position du joueur
	jeu->posy = depy;
	// si le joueur est déplacé sur une cible
	if (jeu->plateau[depx][depy] == CIBLE) {
		jeu->plateau[jeu->posx][jeu->posy] = JOUEUR_CIBLE;
	}
	else {
		jeu->plateau[jeu->posx][jeu->posy] = JOUEUR;
	}
}

/**
* @brief introduit les conditions de déplacement de la caisse
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @param depx type : entier, entrée, case initiale horizontale caisse
* @param depy type : entier, entrée, case initiale verticale caisse
* @param casx type : entier, entrée, case de déplacement horizontale caisse
* @param casy type : entier, entrée, case de déplacement verticale caisse
* @return résultat : la caisse peut/peut pas se déplacer
*/

void deplacer_caisse(t_partie *jeu, int depx, int depy, int casx, int casy){
	// si la caisse est déplacée depuis une cible
	if (jeu->plateau[depx][depy] == CAISSE_CIBLE) {
		jeu->plateau[depx][depy] = CIBLE;
	// si la caisse sur cible est déplacée sur une cible
		if (jeu->plateau[casx][casy] == CIBLE) {
			jeu->plateau[casx][casy] = CAISSE_CIBLE; 
			}
		else {
			jeu->plateau[casx][casy] = CAISSE; 
		}
	}
	// si la caisse est déplacée sur une cible
	else if (jeu->plateau[casx][casy] == CIBLE) {
		jeu->plateau[casx][casy] = CAISSE_CIBLE; 
	}
	else {
		jeu->plateau[casx][casy] = CAISSE; 
	}
}

/**
* @brief cette procédure permet d'annuler les déplacements précédents
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @param posx type : entier, entrée/sortie, position verticale du joueur
* @param posy type : entier, entrée/sortie, position horizontale du joueur
* @param last type : caractère, entrée, stocke le caractère de déplacement
* @return résultat : retourne le perso et/ou caisse sur l'ancien deplacement
*/

void annuler_deplacer(t_partie *jeu, char last){

	int depx = jeu->posx; //case de déplacement horizontale
	int depy = jeu->posy; //case de déplacement verticale
	int casx; // case de déplacement de la caisse
	int casy; 
	int ancienx; // ancienne case de la caisse
	int ancieny; 

	if (last == DEP_HAUT || last == CAISSE_HAUT) {
		depx++; //déplacement vers le Haut
	}
	else if (last == DEP_BAS || last == CAISSE_BAS) {
		depx--; //déplacement vers le Bas
	}
	else if (last == DEP_GAUCHE || last == CAISSE_GAUCHE) {
		depy++; //déplacement à Droite
	}
	else if (last == DEP_DROITE || last == CAISSE_DROITE) {
		depy--; //déplacement à Gauche
	}
	// la case de destination de la caisse correspond a l'ancienne du personnage
	casx = jeu->posx;
	casy = jeu->posy;
	// changement de position du caractère
	deplacer_joueur(jeu, depx, depy);
	// si les anciens déplacements correspondent à celui d'une caisse
	if (last == CAISSE_HAUT ||
		last == CAISSE_BAS ||
		last == CAISSE_GAUCHE ||
		last == CAISSE_DROITE) {
		// ancienne position de la caisse
		ancienx = casx + (casx - jeu->posx);
		ancieny = casy + (casy - jeu->posy);
		// si ancienne position de la caisse = cible
		if (jeu->plateau[ancienx][ancieny] == CAISSE_CIBLE) {
			jeu->plateau[ancienx][ancieny] = CIBLE;
		}
		else {
			jeu->plateau[ancienx][ancieny] = CASE;
		}
		//déplacement de la caisse
		deplacer_caisse(jeu, ancienx, ancieny, casx, casy);
	}
}

/**
* @brief cette procédure contient les touches et conditions pour Analyse.
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @param historiqueDep type : tableau, entrée/sortie, importe le 
	tableau des déplacements
* @param posx type : entier, entrée/sortie, position verticale du joueur
* @param posy type : entier, entrée/sortie, position horizontale du joueur
* @param nbDep type : entier, entrée/sortie, nombre de déplacements
* @param fichier type : chaine, entrée, fichier de sauvegarde
* @return résultat : permet de Analyse au jeu
*/

void Analyse(t_partie *jeu, char fichier[], char deplacements[]){


	char last; // caractère des déplacements du joueur
	int depx = jeu->posx;  // case de déplacement du joueur
	int depy = jeu->posy;

	// scan du caractère du tableau des déplacements
	last = jeu->historiqueDep[jeu->nbDep];
	last = tolower(last); // conversion en minuscule
	// déplacement selon le caractère scanné
		switch (last) {
			case 'h' :
				depx--; //déplacement vers le Haut
				break;
			case 'b' :
				depx++; //déplacement vers le Bas
				break;
			case 'g' :
				depy--; //déplacement à Droite
				break;
			case 'd' :
				depy++; //déplacement à Gauche
				break;
			case 'u' :
				last = jeu->historiqueDep[jeu->nbDep-1]; // stocke le caractère de la case
				annuler_deplacer(jeu, last); 
				break;
			default:
				break;
		}

		if (jeu->plateau[depx][depy] == CAISSE) {
			last = toupper(last); // conversion en majuscule
		}
		// si les touches sont celles de déplacements on utilise les conditions
		
			conditions_dep(jeu, depx, depy, last);
		
		system("clear");
		afficher_entete(jeu, fichier, deplacements);
		afficher_plateau(jeu);
}


/**
* @brief vérifie si il n'y a plus de caisses à déplacer sur les cibles
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @return résultat : retourne le statut de la partie (fini/non fini)
*/

bool gagner (t_partie *jeu) {
	bool win = false; // statut de la partie
	int nbCaisses = 0;
	// compte le nombre de caisses restantes
	for (int lig=0; lig < MAXLIG; lig++) {
		for (int col=0; col < MAXLIG; col++) {
			if (jeu->plateau[lig][col] == CAISSE) {
				nbCaisses++;
			}
		}
	}
	if (nbCaisses == 0) {
		win = true; // toutes les caisses sont sur les cibles
	}
	return win;
}