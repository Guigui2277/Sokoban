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
#define MAXECH 3
#define MINECH 1
#define TAILLE_FICHIER 50

typedef char t_plateau[MAXLIG][MAXLIG];
typedef char t_tabDeplacement[MAXDEP];

//Définition de la structure de jeu
typedef struct{
	int posx; // position horizontale du joueur
	int posy; // position verticale du joueur
	int nbDep; // nombre de déplacements effectués
	int echelle; // taille initiale du plateau
	t_plateau plateau; // déclaration du plateau de jeu
	t_tabDeplacement historiqueDep; // déclaration du tableau des déplacements
} t_partie;


// Définition des caractères constantes.
const char CAISSE = '$';
const char MUR = '#';
const char JOUEUR = '@';
const char CIBLE = '.';
const char JOUEUR_CIBLE = '+';
const char CAISSE_CIBLE = '*';
const char CASE = ' ';

// Définition des touches de déplacement.
const char HAUT = 'z';
const char BAS = 's';
const char GAUCHE = 'q';
const char DROITE = 'd';
const char QUITTER = 'x';
const char RECOMMENCER = 'r';
const char RETOUR = 'u';
const char ZOOMER = '+';
const char DEZOOMER = '-';

// Définition des caractères de déplacement
const char DEP_GAUCHE = 'g';
const char DEP_DROITE = 'd';
const char DEP_HAUT = 'h';
const char DEP_BAS = 'b';
const char CAISSE_GAUCHE = 'G';
const char CAISSE_DROITE = 'D';
const char CAISSE_HAUT = 'H';
const char CAISSE_BAS = 'B';


// liste des procédures déclarées
void chargerPartie(t_plateau plateau, char fichier[]);
void enregistrerPartie(t_plateau plateau, char fichier[]);
void chargerDeplacements(typeDeplacements t, char fichier[], int * nb);
void afficher_entete(t_partie *jeu, char fichier[], char deplacements[]);
void afficher_plateau(t_partie *jeu);
void chercher_joueur(t_partie *jeu);
int kbhit();
void conditions_dep(t_partie *jeu, int depx, int depy, char touche);
void deplacer_joueur(t_partie *jeu, int depx, int depy);
void deplacer_caisse(t_partie *jeu, int depx, int depy, int casx, int casy);
void annuler_deplacer(t_partie *jeu, char last);
bool jouer(t_partie *jeu, char fichier[], char deplacements[]);
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
	jeu.echelle = 1; // définition de l'echelle
	int max; // nombre maximum de caractères dans le tableau déplacements
	char fichier[TAILLE_FICHIER]; // pour stocker le nom du fichier de la partie
	char deplacements[TAILLE_FICHIER]; // pour stocker le nom du fichier des déplacements
	char valider; // pour permettre de valider les enregistrements

	// sélection du niveau
	printf("Quel niveau voulez vous charger ? (ex: niveau1.sok) : ");
	scanf("%s", fichier); // sélection du fichier de la partie
	
	printf("Entrez le nom du fichier de déplacements (ex: niveau1.sok) : ");
	scanf("%s", deplacements); // sélection du fichier des déplacements
	
	
	
	chargerPartie(jeu.plateau, fichier); // charge le fichier
	chargerDeplacements(jeu.historiqueDep, deplacements, &jeu.nbDep);
	afficher_entete(&jeu, fichier, deplacements); 
	afficher_plateau(&jeu);
	chercher_joueur(&jeu);
	printf("Début de l'analyse de la suite...\n");
	// tant qu'il y a des caisses à déplacer
	while (!jouer(&jeu, fichier, deplacements)){
		jouer(&jeu, fichier, deplacements); 
		usleep(500000); // pause de 0.5 seconde
	}

	// affichage des résultats
	if (gagner(&jeu)) {
		printf("La suite de déplacements %s est bien une solution pour la partie %s.\n", fichier, deplacements);
		printf("Elle contient %d déplacements.\n", jeu.nbDep);

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

int max_tab(t_tabDeplacement t){
	int max = 0;
	while (t[max] == '\0'){
		max++;
	}
	return max;
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
* @return résultat : affichage du plateau
*/

void afficher_entete(t_partie *jeu, char fichier[], char deplacements[]){

	system("clear");
	printf(" Nom de la partie : %s\n\n", fichier);
	printf(" Nom du fichier de déplacements : %s\n\n", deplacements);
	printf(" Nombre de déplacement : %d\n\n", jeu->nbDep);
}

/**
* @brief affiche le plateau de jeu
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @param posx type : entier, entrée, taille d'affichage du plateau
* @return résultat : affiche du nombre de caractères selon l'echelle sur plateau
*/

void afficher_plateau(t_partie *jeu){
	char caractere; // caractère de la case correspondante
	char affiche; // affichage de la case

	for (int lig=0; lig < MAXLIG; lig++) {
		for (int ligchar=0; ligchar < jeu->echelle; ligchar++) {
			for (int col=0; col < MAXLIG; col++) {
				caractere = jeu->plateau[lig][col];
				// pour afficher correctement le joueur et la caisse sur cible 
				if (caractere == JOUEUR_CIBLE) {
					affiche = JOUEUR; // joueur_cible affiche joueur
				}
				else if (caractere == CAISSE_CIBLE) {
					affiche = CAISSE; // caisse_cible affiche caisse
				}
				else {
					affiche = caractere;
				}
				for (int colchar=0; colchar < jeu->echelle; colchar++) {
						printf("%c", affiche); 
					}
			}	
			printf("\n"); // nouvelle ligne de tableau
		}
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
* @param touche type : caractère, entrée, importe la touche appuyée
* @return résultat : la caisse peut/peut pas se déplacer
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
				jeu->nbDep++;
			}
		}
		// Uniquement les déplacements du joueur
		else {
			deplacer_joueur(jeu, depx, depy);
			jeu->nbDep++;
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
* @param last type : caractère, entrée, stocke le tableau des déplacements
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
* @brief cette procédure contient les touches et conditions pour jouer.
* @param plateau type : tableau, entrée/sortie, importe le tableau de jeu
* @param historiqueDep type : tableau, entrée/sortie, importe le 
	tableau des déplacements
* @param posx type : entier, entrée/sortie, position verticale du joueur
* @param posy type : entier, entrée/sortie, position horizontale du joueur
* @param nbDep type : entier, entrée/sortie, nombre de déplacements
* @param echelle type : entier, entrée, taille du tableau
* @param fichier type : chaine, entrée, fichier de sauvegarde
* @return résultat : permet de jouer au jeu
*/

bool jouer(t_partie *jeu, char fichier[], char deplacements[]){


	char last; // caractère des déplacements du joueur
	int depx = jeu->posx;  // case de déplacement du joueur
	int depy = jeu->posy;
	bool fini = false; // statut de la partie

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

	// si l'analyse est terminée
	if (jeu->nbDep == max_tab(jeu->historiqueDep)) {
		fini = true; // l'analyse des déplacements est terminée
	}
	return fini;
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