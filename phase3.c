#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NB_CLASSES 6
#define BUFFER_SIZE 50000
#define NB_SEC 1000
#define PATH_NAME_MODEL "E:\\Phase1\\model.csv"
#define PATH_NAME_TRAIN "E:\\Phase1\\trainSet.csv"

typedef struct model Model;
struct model {
	int motionType;
	double averages[NB_SEC];
	int nbValues;
	double stds[NB_SEC];
	double globalAverage;
};

// Signatures
void writeModel(Model model);
void tokenChoice(char toWrite[], int i);
void setTitle(void);
int rowMaker(char toWrite[], int motionType, double values[]);
long int getData(int motionType, double datas[][NB_SEC]);
Model setModel(int i, double datas[][NB_SEC], long int nbValues);

// Main 
void main(void) {
	long int nbValues;

	setTitle();
	
	for(int i = 1; i <= NB_CLASSES;i++){
		double values[100][NB_SEC];
		nbValues = getData(i, values);
		Model model = setModel(i, values, nbValues);
		writeModel(model);
	}
}

// Fonctions
void tokenChoice(char toWrite[], int i) {
	strncat_s(toWrite,BUFFER_SIZE,(i + 1 == NB_SEC ? "\n" : ", "), 1);
}

void setTitle(void) { 
	char toWrite[BUFFER_SIZE] = { "Movement," };
	FILE* fiModel;

	fopen_s(&fiModel, PATH_NAME_MODEL, "w");

	if (fiModel == NULL) printf("Erreur lors de l'ouverture du fichier");
	else {
		for (int i = 0; i < NB_SEC; i++) {
			strncat_s(toWrite, BUFFER_SIZE, "VAcc", 5);
			tokenChoice(toWrite, i);
		}
		
		fwrite(&toWrite, sizeof(toWrite), 1, fiModel);

		fclose(fiModel);
	}
}
int rowMaker(char toWrite[], int motionType, double values[]) {
	char tmp;
	int charCount;
	charCount = sprintf_s(toWrite, BUFFER_SIZE,"%d,", motionType);

	for (int iVAcc = 0; iVAcc < NB_SEC; iVAcc++) {
		charCount += sprintf_s(toWrite + charCount, BUFFER_SIZE - charCount, "%f", values[iVAcc]);
		tokenChoice(toWrite, iVAcc);
		charCount++;
	}

	return charCount;
}

long int getData(int motionType, double datas[][NB_SEC]) {
	int nbRows = 0;
	int nbColumns = 0;
	char class;
	char caracLu;
	char toWrite[NB_SEC] = { "" };
	long int nbValues = 0;

	sprintf_s(&class, BUFFER_SIZE, "%d", motionType);

	FILE* fiTrain;

	fopen_s(&fiTrain, PATH_NAME_TRAIN, "r");

	if (fiTrain == NULL) printf("erreur lors de l'ouverture du fichier");
	else {
		while (!feof(fiTrain)) {
			fread_s(&caracLu, sizeof(caracLu), sizeof(caracLu), 1, fiTrain);
			if (caracLu == class) {
				int i = 0;
				// Avancer jusqu'au VAcc
				while(i < 3) {
					fread_s(&caracLu, sizeof(caracLu), sizeof(caracLu), 1, fiTrain);
					if (caracLu == ',') i++;
				}
				// Enregistrer la ligne
				while (caracLu != '\n') {
					fread_s(&caracLu, sizeof(char), sizeof(char), 1, fiTrain);
					
					if (caracLu != ',') {
						strncat_s(toWrite,BUFFER_SIZE, &caracLu, 1);
					}
					else {
						char* tmp;
						datas[nbRows][nbColumns] = strtod(toWrite, &tmp); //tmp doit etre vide 
						nbValues++;
						nbColumns++;
						strcpy_s(toWrite,BUFFER_SIZE,"");
					}
				}
				nbRows++;
				nbColumns = 0;
			}
			else // Passage à la ligne suivante si le motionType ne correspond pas
				while(caracLu != '\n')
					fread_s(&caracLu, sizeof(caracLu), sizeof(caracLu), 1, fiTrain);
		}
		fclose(fiTrain);
	}

	return nbValues;
}

Model setModel(int i, double datas[][NB_SEC], long int nbValues) {
	Model model;
	double totalValue = 0;
	double stds = 0;
	int nbRows;
	model.motionType = i;
	model.nbValues = nbValues ;
	nbRows = model.nbValues / NB_SEC;

	for (int iColonne = 0; iColonne < NB_SEC; iColonne++) {
		double average = 0;
		// Récupère chaque VAcc pour chaque dixième de seconde
		for (int iRow = 0; iRow < nbRows; iRow++) {
			average += datas[iRow][iColonne];
		}

		totalValue += average;
		//Moyenne par dixième de seconde
		model.averages[iColonne] = average / nbRows;

		// Calcul de l'écart type par dixieme de seconde
		for (int iRow = 0; iRow < nbRows; iRow++) {
			// Calcul de la somme des carré de l'écart à la moyenne de chacune des valeurs
			stds += pow(model.averages[iColonne] - datas[iRow][iColonne], 2);
		}
		// Division par le nombre de valeurs
		stds /= nbRows;
		stds = sqrt(stds);

		model.stds[iColonne] = stds;
	}
	model.globalAverage = (double)totalValue / model.nbValues;

	return model;
}

void writeModel(Model model) {
	char toWrite[BUFFER_SIZE];
	FILE* fiModel;

	fopen_s(&fiModel, PATH_NAME_MODEL, "a+"); // Ouverture en écriture, pointeur a la fin du fichier

	if(fiModel == NULL) printf("Erreur lors de l'ouverture du fichier");
	else {
		int charCount;
		//char tmp[10];

		// Ajout de la partie moyenne
		charCount = rowMaker(toWrite, model.motionType, model.averages);
		fwrite(&toWrite, charCount, 1, fiModel);
		strncpy_s(toWrite, sizeof(char), "", 1);

		// Ajout de la partie écarts type
		charCount = rowMaker(toWrite, model.motionType, model.stds);
		fwrite(&toWrite, charCount, 1, fiModel);
		strncpy_s(toWrite, sizeof(char), "",1);

		charCount = sprintf_s(toWrite, BUFFER_SIZE, "%d,", model.motionType);
		charCount += sprintf_s(toWrite + charCount, BUFFER_SIZE - charCount, "%f\n", model.globalAverage);
		//strncat_s(toWrite, BUFFER_SIZE - charCount, tmp, 10);

		fwrite(&toWrite, charCount,1, fiModel);

		fclose(fiModel);
	}
}
