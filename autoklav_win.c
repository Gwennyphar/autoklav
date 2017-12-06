/**
 * Programname : autoklav.c
 * Autor       : Nico Anders
 * Version     : 1.0
 * Stand       : 29.12.2014
 * Status      : in Arbeit
 */

/**
 * Praeprozessor
 */
 #include <stdio.h>
 #include <stdlib.h>
 #include <dirent.h>
 #include <sys/types.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <string.h>
 #include <time.h>
 #include <locale.h>
 #include <errno.h>
 #ifdef __linux__
	#define clr(); system("clear");
	#define oe "�"
 #else
	#define clr(); system("cls");
	#define oe "\x94"
 #endif

/**
 * Funktion : globale Variable zum uebergeben des eingegeben Ordnernamen
 * Status   : vorgelegt
 */
 struct Daten
 {
	 char sPfad[256];
	 char sDateiname[256];
	 int  iDruck;
 }stEingabe;


/**
 * Funktion : ruft de Hilfeseite auf
 * Status   : vorgelegt
 */
 int iHilfe_anzeigen() {
	 clr();
	 printf("\n\tA U T O K L A V  P R O T O K O L L E R\n"
	 "\t======================================\n"
	 "\n\t\t     H I L F E\n"
	 "\t______________________________________\n"
	 "\n\tKopieren Sie das Programm in den"
	 "\n\tHauptordner, wo die Unterordner mit"
	 "\n\tden Autoklavdateien enthalten sind."
	 "\n\tGeben Sie den Namen des Ordners ein,"
	 "\n\tum in den Unterordner zu gelangen."
	 "\n\tDas Programm kopiert automatisch"
	 "\n\talle Protokolle und speichert den"
	 "\n\tInhalt als ausdruckbare Textdatei"
	 "\n\toder kann direkt ausgedruckt werden."
	 "\n\tSind in dem Ordner keine Dateien"
	 "\n\tenthalten, wird dies, wie alle"
	 "\n\tanderen Schritte auch, mitgeteilt."
	 "\n\tMit der Taste A werden alle Ordner"
	 "\n\tangezeigt oder ausgeblendet."
	 "\n\t______________________________________\n"
	 "\tWeiter mit Enter...");
	 fgetc(stdin);
	 return EXIT_SUCCESS;
 }

/**
 * Funktion : ruft de Infoseite auf
 * Status   : vorgelegt
 */
 int iInfo_anzeigen()
 {
	 time_t tDatum;
	 struct tm *stDatum;
	 char sJahr[20];
	 /* aktuelles Jahr */
	 time (&tDatum);
	 stDatum = localtime (&tDatum);
	 strftime(sJahr, sizeof(sJahr), "%Y", stDatum);
	 clr();
	 printf("\n\tA U T O K L A V  P R O T O K O L L E R\n"
	 "\t======================================\n"
	 "\n\tAutor        : Nico Anders"
	 "\n\tVersion      : 1.0"
	 "\n\tDatum        : 27.12.2014"
	 "\n\tContact      : nicoanders@freenet.de"
	 "\n\tCopyright    : (c)%s"
	 "\n\t______________________________________\n"
	 "\tWeiter mit Enter...", sJahr);
	 fgetc(stdin);
	 return EXIT_SUCCESS;
 }

/**
 * Funktion : ersetzt Zeichen im String
 * Status   : vorgelegt
 */
 char * stringReplace(char *search, char *replace, char *string)
 {
	 char *tempString, *searchStart;
	 int len=0;

	 /* pruefe ob Such-String vorhanden ist */
	 searchStart = strstr(string, search);
	 if(searchStart == NULL)
	 {
		 return string;
	 }
	 /* Speicher reservieren */
	 tempString = (char*) malloc(strlen(string) * sizeof(char));
	 if(tempString == NULL)
	 {
		 return NULL;
	 }
	 /* temporaere Kopie anlegen */
	 strcpy(tempString, string);
	 /* ersten Abschnitt in String setzen */
	 len = searchStart - string;
	 string[len] = '\0';
	 /* zweiten Abschnitt anhaengen */
	 strcat(string, replace);
	 /* dritten Abschnitt anhaengen */
	 len += strlen(search);
	 strcat(string, (char*)tempString+len);
	 /* Speicher freigeben */
	 free(tempString);
	 return string;
 }

/**
 * Funktion : sendet Datei an Drucker
 * Status   : vorgelegt
 */
 int iDrucke_Datei()
 {
	 FILE *FPLesen;
	 FILE *FPDrucken;
	 FPDrucken = popen("lp", "w");
	 if(FPDrucken != NULL ) {
		 FPLesen = fopen(stEingabe.sDateiname, "r");
		 do {
			 /*Inhalt lesen */
			 fgets(stEingabe.sDateiname,
			 sizeof(stEingabe.sDateiname), FPLesen);
			 /* drucken */
			 fprintf(FPDrucken, "%s", stEingabe.sDateiname);
		 }while(!feof(FPLesen));
		 fclose(FPLesen);
		 pclose(FPDrucken);
	 } else {
		 perror("\tKein Drucker angeschlossen!\n");
		 return EXIT_FAILURE;
	 }
	 stEingabe.iDruck = 0;
	 return EXIT_SUCCESS;
 }

/**
 * Funktion : Kopiert Inhalte aus versch. Dateien und speichert diesen in
 * eine Ausgabedatei
 * Status   : vorgelegt
 */
 int iKopiere_Dateien()
 {
	 FILE *FPLesen;
	 FILE *FPSchreiben;
	 time_t tDatum;
	 DIR *FPVerzeichnis;
	 char *sDatum;
	 char *cZeichen;
	 char search[] = "Str�m";
	 char replace[] = "Stroem";
	 struct dirent *FPDateien;
	 char sQuelldatei[FILENAME_MAX][256];
	 char sZieldatei[5000][256];
	 int iPos_Datei = 0;                      /* Indexzaehler fuer Dateiname */
	 int iZeilen = 0;                         /* Indexzaehler fuer Textzeilen */
	 int iAnzahl = 0;                         /* Anzahl der Dateien */
	 int iGesamt = 0;                         /* Anzahl aller Zeilen */
	 /* Verzeichnis oeffnen */
	 FPVerzeichnis = opendir(stEingabe.sPfad);
	 if(FPVerzeichnis != NULL) {
		 /* Quelldateien kleben */
		 while((FPDateien = readdir(FPVerzeichnis)) != NULL) {
			 /*nur Dateiendung mit PRO auflisten */
			 if(strstr(FPDateien->d_name, ".PRO")) {
				 snprintf(sQuelldatei[iAnzahl], sizeof(sQuelldatei[iAnzahl]),
				 "./%s/", stEingabe.sPfad);
				 strncat(sQuelldatei[iAnzahl], FPDateien->d_name,
				 sizeof(sQuelldatei[iAnzahl]));
				 iAnzahl++;
			 }
		 }		 
		 if(iAnzahl != 0) {
			 /* Zeitstempel setzen */
			 time(&tDatum);
			 sDatum = ctime(&tDatum);
			 /* Zeilenumbruch entfernen */
			 sDatum[strlen(sDatum)-1] = '\0';
			 /* Doppelpunkte ersetzen */
			 while((cZeichen = strchr(sDatum, ':'))!=NULL) {
				 *cZeichen = '-';
			 }
			 /* Zieldatei kleben */
			 snprintf(stEingabe.sDateiname,
			 sizeof(stEingabe.sDateiname), "./%s/", stEingabe.sPfad);
			 strncat(stEingabe.sDateiname, "protokoll_",
			 sizeof(stEingabe.sDateiname));
			 strncat(stEingabe.sDateiname, sDatum,
			 sizeof(stEingabe.sDateiname));
			 strncat(stEingabe.sDateiname, ".txt",
			 sizeof(stEingabe.sDateiname));
		 }
		 closedir(FPVerzeichnis);
	 }
	/****************** kopieren ************************/
	for(iPos_Datei = 0; iPos_Datei < iAnzahl; iPos_Datei++) {
		/* Datei lesend oeffnen */
		FPLesen = fopen(sQuelldatei[iPos_Datei], "r");
		if(FPLesen != NULL) {
			do {
				/*Inhalt lesen */
				fgets(sQuelldatei[iPos_Datei],
				sizeof(sQuelldatei[iPos_Datei]), FPLesen);
				/* Inhalt kopieren */
				strncpy(sZieldatei[iGesamt], sQuelldatei[iPos_Datei],
				sizeof(sZieldatei[iGesamt]));
				iGesamt++;
			}while(!feof(FPLesen));
			fclose(FPLesen);
		}	
	}
	/*********************** speichern ******************************/
	 FPSchreiben = fopen(stEingabe.sDateiname, "w");
	 if(iAnzahl == 0) {
		 printf("\tKeine Protokolle vorhanden!\n");
		 printf("\tWeiter mit Enter...");
		 fgetc(stdin);
		 return EXIT_FAILURE;
	 } else if(FPSchreiben == NULL) {
		 printf("\t%s nicht beschreibar!\n", stEingabe.sDateiname);
		 printf("\t --> (%s)\n", strerror(errno));
		 printf("\tWeiter mit Enter...");
		 fgetc(stdin);
		 return EXIT_FAILURE;
	 } else {
		 for(iZeilen = 0; iZeilen < iGesamt; iZeilen++ ) {
			 /* Sonderzeichen aus String entfernen */
			 stringReplace(search, replace, sZieldatei[iZeilen]);
			 fputs(sZieldatei[iZeilen], FPSchreiben);			 
		 }
		 fclose(FPSchreiben);
		 printf("\t%s efolgreich kopiert!\n", stEingabe.sDateiname);

		 /*********************** drucken ******************************/
		 stEingabe.iDruck = 1;
		 //iDrucke_Datei();
	 }
	 printf("\tWeiter mit Enter...");
	 fgetc(stdin);
	 return EXIT_SUCCESS;
 }

/**
 * Steuerprogramm
 */
 int iController()
 {
	 DIR *FPVerzeichnis;
	 struct dirent *FPDateien;
	 struct stat attribut;
	 int i = 0;										 /* Ordner ein/ausblenden */
	 /* Frage Druckstatus ab */
	 if(stEingabe.iDruck != 1) {
		 stEingabe.iDruck = 0;
	 }
	 do {
		 clr();
		 printf("\n\tA U T O K L A V  P R O T O K O L L E R\n"
		 "\t======================================\n"
		 "\n\tA = Ordner anzeigen   |   H = Hilfe"
		 "\n\tB = Beenden           |   I = Info\n"
		 "\t______________________________________\n");
		 FPVerzeichnis = opendir(".");
		 if((stEingabe.sPfad[0] == 'a' ||
		 stEingabe.sPfad[0] == 'A') && i == 1) {
			 i = 0;
		 } else if(stEingabe.sPfad[0] == 'a' ||
		 stEingabe.sPfad[0] == 'A' || i == 1) {
			 if(FPVerzeichnis != NULL) {
				 while((FPDateien = readdir(FPVerzeichnis)) != NULL) {
					 if(stat(FPDateien->d_name, &attribut) == -1) {
						 printf(" --> (%s)\n", strerror(errno));
						 return EXIT_FAILURE;
					 }
					 /* Dateiart erfragen */
					 if( S_ISREG(attribut.st_mode)) {
						 printf("\tDatei            : ");
					 } else if( S_ISDIR(attribut.st_mode)) {
						 printf("\tVerzeichnis      : ");
					 } else {
						 printf("\tUnbekannt        : ");
					 }
					 /* Dateinamen ausgeben */
					 printf("%-20s\n",FPDateien->d_name);
				 }
				 closedir(FPVerzeichnis);
				 i = 1;
				 printf("\t_____________________________________\n");
			 }
		 }
		 if(stEingabe.iDruck == 1) {
			 printf("\n\tM%schten Sie drucken? j/n ", oe);
		 } else {
			 printf("\n\tVerzeichnis wechseln: ");
		 }
		 fgets(stEingabe.sPfad, sizeof(stEingabe.sPfad), stdin);
		 stEingabe.sPfad[strlen(stEingabe.sPfad) -1] = '\0';
		 if( (stEingabe.sPfad[0] == 'j' || stEingabe.sPfad[0] == 'J') &&
		 stEingabe.iDruck == 1) {
			 iDrucke_Datei();
		 } else if( (stEingabe.sPfad[0] == 'n' || stEingabe.sPfad[0] == 'N') &&
		 stEingabe.iDruck == 1 ) {
			 stEingabe.iDruck = 0;
			 continue;
		 }else if(stEingabe.sPfad[0] == 'i' || stEingabe.sPfad[0] == 'I') {
			iInfo_anzeigen();
			continue;
		 } else if(stEingabe.sPfad[0] == 'h' || stEingabe.sPfad[0] == 'H') {
			iHilfe_anzeigen();
			continue;
		 } else if(stEingabe.sPfad[0] == 'b' || stEingabe.sPfad[0] == 'B') {
			 break;
		 } else if(
		 stEingabe.sPfad[0] != 'a' &&
		 stEingabe.sPfad[0] != 'A' &&
		 stEingabe.sPfad[0] != '\0') {
			 /* Jetzt wollen wir in das neue Verzeichnis wechseln. */
			 if((FPVerzeichnis = opendir(stEingabe.sPfad)) == NULL) {
				 printf("\tKonnte nicht nach '%s' wechseln\n", stEingabe.sPfad);
				 printf("\t --> (%s)\n", strerror(errno));
				 printf("\tWeiter mit Enter...");
				 fgetc(stdin);
			 } else {
				 printf("\tErfolgreich nach '%s' gewechselt!\n",
				 stEingabe.sPfad);
				 printf("\t --> (%s)\n", strerror(errno));
				 iKopiere_Dateien();
			 } 
		 }
	 } while(stEingabe.sPfad[0] != 'b');
	 printf("\tProgamm beendet!\n");
	 return EXIT_SUCCESS;
 }

/**
 * Hauptprogramm
 */
 int main()
 {
	 iController();
	 return EXIT_SUCCESS;
 }
