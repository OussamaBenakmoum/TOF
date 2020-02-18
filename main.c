/**************************************************************************
 * Exemple de programme C manipulant un fichier TOF / SFSD 2019 / Hidouci *
 **************************************************************************/



#include "modele.h"
#include <stdio.h>




FILE *f = NULL;
t_entete ent;
tbloc buf;

void charg();		// Chargement initial du fichier
void reorg();		// R�organisation du fichier
void info();		// Affichage de l'ent�te
void rech();		// Recherche d'un enregistrement :
long dicho( long val, int *trouv, long *i, int *j ); // par dichotomie ou
long seq( long val, int *trouv, long *i, int *j );   // par recherche s�quentielle
void ins();		// Insertion par d�calages d'un enregistrement
void sup();		// Suppression logique d'un enregistrement
void parcours();  	// Affichage d'une s�quence de blocs contigus (entre a et b)



int main()
{
   int choix , i;
   char nom[20], mode[20];
   printf("le nombre de blocs :  %d \n",ent.nb_bloc);
   printf("Op�rations d'acc�s sur un Fichier de type TOF\n");
   printf("Capacit� maximale des blocs = %d enregistrements\t", MAXTAB);
   printf("Taille d'un bloc = %ld \tTaille entete = %ld\n\n", sizeof(tbloc), sizeof(t_entete) );

   // Ouverture du fichier ...
   printf("Donnez le nom du fichier : ");
   scanf(" %s", nom);
   printf("Ancien ou Nouveau ? (a/n) : ");
   scanf(" %s", mode);
   if ( mode[0] == 'a' || mode[0] == 'A' )
	ouvrir( &f, nom , 'A', &ent );
   else {
	ouvrir( &f, nom , 'N', &ent );
	charg();
   }

   // Menu principal ...
   do {
      	printf("\n--------- M E N U ---------\n");
	printf("1) Afficher l'entete du fichier\n");
	printf("2) Rechercher un enregistrement dans le fichier\n");
	printf("3) Inserer un enregistrement dans le fichier\n");
	printf("4) Supprimer un enregistrement dans le fichier\n");
	printf("5) Affichage de bloc(s)\n");
	printf("6) R�organisation du fichier\n");
	printf("0) Quitter le programme\n");
	printf("\tchoix : ");
	scanf(" %d", &choix);
	printf("\n---------------------------\n\n");

	switch(choix) {
	   case 1: info(); break;
	   case 2: rech(); break;
	   case 3: ins(); break;
	   case 4: sup(); break;
	   case 5: parcours(); break;
	   case 6: reorg(); break;
	}
   } while ( choix != 0);

   // Fermeture du fichier (sauvegarde de l'entete) ...
   fermer(f, &ent);

   return 0;

} // main



// Chargement initial � u% ...
void charg()
{
   double u;
   long i, k, n;
   int j;

   printf("Chargment initial du fichier\n");
   printf("Donnez le nombre d'enregistrement � ins�rer : ");
   scanf(" %ld", &n);
   printf("Donnez le taux de chargement souhait� (entre 0 et 1) : ");
   scanf(" %lf", &u);
   if ( u < 1/MAXTAB ) u = 1/MAXTAB; // pour avoir au moins un enregistrement par bloc
   if ( u > 1 ) u = 1.0;	     // maximum = 100%

   printf("Remplissage des blocs avec %d enreg (sauf �ventuellement le dernier)\n",\(int)(MAXTAB*u));
   j = 0;
   i = 1;
   for (k=0; k<n; k++) {
	if ( j < MAXTAB*u ) {
	   buf.tab[j] = k*10;
	   buf.eff[j] = ' ';
	   j++;
	}
	else {
	   buf.nb = j;
	   ecrireDir(f, i, &buf);
	   i++;
	   buf.tab[0] = k*10;
	   buf.eff[0] = ' ';
	   j = 1;
	}
   }
   // derni�re �criture ...
   buf.nb = j;
   ecrireDir(f, i, &buf);
   // m-a-j de l'entete ...
   ent.nb_bloc = i;
   ent.nb_ins = n;

} // charg


// r�oragnisation du fichier ...
// c-a-d chargement initial d'un nouveau fichier � partir des enreg de l'ancien fichier
void reorg()
{
   double u;
   long i, i2;
   int j, j2;

   // d�claration du nouveau fichier � construire
   char nom2[20];	// nom du nouveau fichier
   tbloc buf2;		// buffer utilis� pour sa construction
   t_entete ent2;	// entete
   FILE *f2;		// variable fichier C

   printf("r�organisation du fichier\n");
   printf("Donnez le nom du nouveau fichier � construire : ");
   scanf(" %s", nom2);

   ouvrir( &f2, nom2, 'N', &ent2 );

   printf("Donnez le taux de chargement souhait� dans le nouveau fichier (entre 0 et 1) : ");
   scanf(" %lf", &u);
   if ( u < 1/MAXTAB ) u = 1/MAXTAB;
   if ( u > 1 ) u = 1.0;

   printf("Remplissage des blocs avec %d enreg (sauf �ventuellement le dernier)\n",\
	   (int)(MAXTAB*u));

   // parcours s�quentiel de l'ancien fichier (f) pour charger le nouveau fichier (f2)
   // avec les enregistrements de f non effac�s logiquement

   j2 = 0;
   i = i2 = 1;
   while ( i <= ent.nb_bloc ) {
	lireDir( f, i, &buf );
	for (j=0; j<buf.nb; j++)
	   if ( buf.eff[j] == ' ' ) {
		// si l'enreg n'est pas effac�, on l'ins�re dans buf2
		buf2.tab[j2] = buf.tab[j];
		buf2.eff[j2] = ' ';
		j2 = j2 + 1;
		if ( j2 > MAXTAB*u ) {
		   // si buf2 est plein � plus de u%, on le vide sur disque
		   buf2.nb = j2;
	   	   ecrireDir(f2, i2, &buf2);
	   	   i2 = i2 + 1;
	   	   j2 = 0;
		}
	   }
	i++;
   }

   // derni�re �criture (si buf2 n'est pas vide) ...
   if ( j2 > 0 ) {
	buf2.nb = j2;
 	ecrireDir( f2, i2, &buf2 );
   }

   // m-a-j de l'entete de f2 ...
   if ( j2 > 0 )
	ent2.nb_bloc = i2;
   else
	ent2.nb_bloc = i2 - 1;
   ent2.nb_ins = ent.nb_ins - ent.nb_sup;
   ent2.nb_sup = 0;

   // r�ouverture du nouveau fichier avec les varaibles globales du prog: f, buf et ent
   fermer( f2, &ent2 );
   fermer( f , &ent );
   ouvrir( &f, nom2, 'A', &ent );

} // reorg


// afficher l'entete du fichier ...
void info()
{
   printf("Informations sur le fichier \n");
   printf("\tNombre de blocs utilis�s = %ld\n", ent.nb_bloc);
   printf("\tNombre d'enregistrements ins�r�s = %ld\n", ent.nb_ins);
   printf("\tNombre d'enregistrements supprim�s = %ld\n", ent.nb_sup);
   printf("\tFacteur de chargement moyen = %ld\n", \
	(ent.nb_bloc == 0 ? 0 : (long)(((double)ent.nb_ins / (ent.nb_bloc*MAXTAB))*100)) );
} // info


// recherche d'une valeur dans le fichier:
// utilise soit la recherche dichotomique, soit la recherche s�quentielle
void rech()
{
   long i, val, cpt;
   int trouv, j, choix;

   printf("Recherche 1-dichotomique ou 2-s�quentielle ? (1 ou 2) : ");
   scanf(" %d", &choix);
   printf("Donnez la valeur cherch�e : ");
   scanf(" %ld", &val);

   if ( choix == 1 )
	cpt = dicho( val, &trouv, &i, &j );
   else
	cpt = seq( val, &trouv, &i, &j );

   printf("R�sultats de la recherche:\n");
   if (trouv && buf.eff[j] == ' ')
	printf("�l�ment trouv� dans le bloc %ld � la position %d\n", i, j);
   else
	printf("�l�ment non trouv� et devrait �tre dans le bloc %ld � la position %d\n", i, j);
   printf("Nombre de lectures physiques effectu�es = %ld\n", cpt);

} // rech


// recherche dichotomique
// r�sultats: trouv (bool�en), i (num de bloc), j (position dans le bloc)
// retourne aussi le nombre de lecture de blocs effectu�s.
long dicho( long val, int *trouv, long *i, int *j )
{
   long bi, bs, cpt;
   int stop, inf, sup;

   bi = 1;
   bs = ent.nb_bloc;
   *trouv = 0;
   stop = 0;
   cpt = 0; // compteur de lectures physiques
   while ( bi <= bs && !*trouv && !stop ) {
	*i = (bi + bs) / 2;		// le milieu entre bi et bs
	lireDir( f, *i, &buf ); cpt++; 	// lecture du bloc du milieu
	if ( val < buf.tab[0] )
	   bs = *i - 1;		// la recherche contiue dans la 1ere moiti�
	else
	   if ( val > buf.tab[ buf.nb-1 ] )
	      	bi = *i + 1;	// la recherche contiue dans la 2eme moiti�
	   else {
		   stop = 1;	// recherche interne dans buf (bloc du milieu)
	   	inf = 0;
		   sup = buf.nb-1;
		   while ( inf <= sup && !*trouv ) {
		      *j = (inf + sup) / 2;
		      if ( val == buf.tab[*j] ) *trouv = 1;
		      else
			   if ( val < buf.tab[*j] )
			      sup = *j - 1;
			   else
			   inf = *j + 1;
	   	   } // while interne
		if ( inf > sup ) *j = inf;
	   }
   } // while externe
   if ( bi > bs ) {
	*i = bi;
	*j = 0;
   }

   return cpt;

} // dicho


// recherche sequentielle
// r�sultats: trouv (bool�en), i (num de bloc), j (position dans le bloc)
// retourne aussi le nombre de lecture de blocs effectu�s.
long seq( long val, int *trouv, long *i, int *j )
{
   long cpt;
   int stop;

   *i = 1;
   *trouv = 0;
   stop = 0;
   cpt = 0; // compteur de lectures physiques
   while ( *i <= ent.nb_bloc && !*trouv && !stop ) {
	lireDir( f, *i, &buf ); cpt++;	// lecture physique
	*j = 0;
	while ( *j < buf.nb && !stop && !*trouv ) {
	   if ( val == buf.tab[*j] ) *trouv = 1;
	   else
		if ( val < buf.tab[*j] )
			stop = 1;
		else
			*j = *j + 1;
	} // while interne
	if ( !stop && !*trouv )
	   *i = *i + 1;
   } // while externe

   return cpt;

} // seq


// Insertion par d�calages en r�utilisant les emplacements des enreg effac�s logiquement
void ins()
{
   long i, val, sauv, cptR, cptI;
   int trouv, continu, empl_recup, j;

   printf("Insertion d'un enregistrement\n");
   printf("Donnez la valeur � ins�rer : ");
   scanf(" %ld", &val);

   // recherche de la position (i,j) o� ins�rer l'enregistrement
   cptR = dicho( val, &trouv, &i, &j );

   printf("La recherche a co�t� %ld lectures physiques.\n", cptR);
   if (trouv && buf.eff[j] == ' ') {
      // si l'enreg existe et n'est pas effac�e logiquement, on arrete l'insertion.
      printf("Insertion refus�e car la valeur existe d�j� dans le bloc %ld � la position %d\n"\
	      , i, j);
      return;
   }

   printf("La valeur doit �tre ins�rer dans le bloc %ld � la position %d", i, j);
   printf(" ... "); fflush(stdout);

   cptI = 0;	// compteur de lecture/ecriture dans la phase de d�calages

   empl_recup = 0; // indicateur de r�utilisation d'un emplacement effac� logiquement
		   // lors de l'insertion de la nouvelle valeur

   if ( i > ent.nb_bloc ) {
      // cas particulier d'une insertion en fin de fichier ...
      buf.tab[0] = val;
      buf.eff[0] = ' ';
      buf.nb = 1;
      ecrireDir( f, i, &buf ); cptI++;
      ent.nb_bloc++;
   }
   else {
      // cas g�n�ral ...
      continu = 1;
      while ( continu ) {	// boucle principale pour les decalages inter-blocs
	while ( continu && j < buf.nb ) {	// d�calages intra-bloc ...
	   sauv = buf.tab[j];
	   buf.tab[j] = val;
	   if ( buf.eff[j] == '*' ) {
		continu = 0;
		buf.eff[j] = ' ';
	   }
	   else {
	   	j++;
		val = sauv;
	   }
	}

	if ( j == buf.nb ) 	// ou continu == 1
	   if ( buf.nb < MAXTAB ) {
		buf.nb++;
		buf.tab[j] = val;  buf.eff[j] = ' ';
		ecrireDir( f, i, &buf ); cptI++;
		continu = 0;
	   }
	   else {
		ecrireDir( f, i, &buf ); cptI++;
		i++;  j = 0;
		if ( i <= ent.nb_bloc ) {
		   lireDir( f, i, &buf ); cptI++;
		}
		else {
      	      	   // on a d�pass� la fin de fichier, donc on rajoute un nouveau bloc ...
      	      	   buf.tab[0] = val;
      	      	   buf.eff[0] = ' ';
      	      	   buf.nb = 1;
      	      	   ecrireDir( f, i, &buf ); cptI++;
     	      	   ent.nb_bloc++;
	      	   continu = 0; // et on s'arr�te
		}
	   }
	else {
	   // cas o� les d�calages se sont arr�t�s sur un enreg effac� logiquement
	   empl_recup = 1;
	   ent.nb_sup--;	// car on r�utilis� l'emplacement effac� logiquement
	   ecrireDir( f, i, &buf ); cptI++;
	}

      } // fin de la boucle while principale

   } // else (cas g�n�ral)

   if ( !empl_recup )		// si on n'a pas reutilis� un emplacement effac� logiq
      ent.nb_ins++; 		// alors on incr�mente le nombre d'insertions

   printf("\nInsertion termin�e. \n");
   printf("\tCo�t en op�rations de lecture/�criture (rech:%ld + decl:%ld) = %ld\n", \
		cptR, cptI, cptR+cptI);

} // ins


// Suppression logique d'un enregistrement
void sup()
{
   long i, val, cptR;
   int trouv, continu, j;

   printf("Suppression (logique) d'un enregistrement\n");
   printf("Donnez la valeur � suprimer : ");
   scanf(" %ld", &val);

   cptR = dicho( val, &trouv, &i, &j );

   printf("La recherche a co�t� %ld lectures physiques.\n", cptR);
   if (!trouv || buf.eff[j] == '*') {
      printf("Suppression refus�e car la valeur ,n'existe pas\n");
      return;
   }

   printf("La valeur � supprimer est dans le bloc %ld � la position %d\n", i, j);

   buf.eff[j] = '*';		// effacement logique de l'enregistrement
   ecrireDir( f, i, &buf );

   ent.nb_sup++;

   printf("Co�t total (rech + sup) = %ld + 1 = %ld op. d'E/S\n", cptR, cptR+1 );

} // sup


// Affichage d'une s�quence de blocs contigus (entre a et b)
void parcours()
{
   long i, a, b;
   int j;

   printf("Affichage d'une s�quence de bloc(s). ayant des adresses dans [a , b] \n");
   printf("le fichier commence dans le bloc num 1 et se termine dans le bloc num %ld\n", ent.nb_bloc);
   printf("donnez a : ");
   scanf(" %ld", &a);
   printf("donnez b : ");
   scanf(" %ld", &b);
   for (i = a; i <= b; i++) {
      lireDir(f, i, &buf);
      printf("[Bloc Num:%3ld \tNB = %2d \tCapacit� max = %2d]\n", i, buf.nb, MAXTAB);
      for( j=0; j<buf.nb; j++)
	 if ( buf.eff[j] == ' ' )
	    printf("%ld ", buf.tab[j]);
	 else
	    printf("*%ld* ", buf.tab[j]);
      printf("\n--------------------------------------------------\n");
   }

} // parcours




