#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <set>
#include <limits>

using namespace std;

ifstream fin("royfloyd.in");
ofstream fout("royfloyd.out");

//---------------------------------COD TEMA 1--------------------------------------------------------------------------------

class Graf
{
protected:
	int nrMuchii, nrNoduri;
	vector<vector<int > > listaAdiacenta;
	bool orientat;
private:
	void adaugaMuchie(int, int, bool);
	//Metoda auxiliara folosita in NrCompConexeDFS
	vector<bool> DFSAux(int, vector<bool>);
	//Metoda auxiliara folosita in Biconexe()
	void BCXAux(int, int&, vector<int>&, vector<int>&, vector<int>&, stack<int>&, vector<vector<int>>&);
	//Metoda auxiliara folosita in CTC()
	void CTCAux(int, int&, vector<int>&, vector<int>&, vector<bool>&, stack<int>&, vector<vector<int>>&);
	//Metoda auxiliara folosita in MuchiiCrit()
	void MuchiiAux(int, int&, vector<int>&, vector<int>&, vector<int>&, vector<bool>&, vector<vector<int>>&);
	//Metoda auxiliara pt Sortarea topologica
	void DFSTopological(int, vector<bool>&, stack<int>&);	

	friend void countSort(vector<int>&);
public:
	Graf();
	Graf(int, int, bool);
	void setListaAdiacenta();
	vector<int> BFS(int);					//Metoda care rezolva problema BFS de pe infoarena
	int NrCompConexeDFS(int);				//Metoda care rezolva problema DFS de pe infoarena
	vector<vector<int>> Biconexe();			//Metoda care rezolva problema Componente Biconexe de pe infoarena
	bool HavelHakimi();						//Metoda care pune in practica Havel Hakimi	
	vector<vector<int>> CTC();				//Metoda care afiseaza componentele tare conexe ale grafului	
	vector<vector<int>> MuchiiCrit();		//Metoda care afiseaza muchiile critice ale grafului
	stack<int> SortareTopologica();			//Metoda care sorteaza topologic nodurile grafului
	int DiametruArbore();					//Metoda care determina diametrul arborelui
};

Graf::Graf()
{
	nrMuchii = 0;
	nrNoduri = 0;
	orientat = false;
}

Graf::Graf(int n, int m, bool orr)
{

	nrNoduri = n;
	nrMuchii = m;
	orientat = orr;
	
}

void Graf::setListaAdiacenta()
{
	int x, y;

	listaAdiacenta.resize(nrNoduri + 1);	

	for (int i = 0; i < nrMuchii; i++)
	{
		fin >> x >> y;
		adaugaMuchie(x, y, orientat);
	}
}

void Graf::adaugaMuchie(int n1, int n2, bool orr)
{
	listaAdiacenta[n1].push_back(n2);
	if (!orr)
	{
		listaAdiacenta[n2].push_back(n1);
	}
}

// Resume BFS:
// 
// Fiind dat un nod S, se determina,
// pentru fiecare nod X, numarul minim de arce ce trebuie 
// parcurse pentru a ajunge din nodul sursa S la nodul X
// 
// Mod functionare:
// 
// Folosim BFS si retinem distantele intr-un vector
// Apoi le afisam

vector<int> Graf::BFS(int nodStart)
{
	queue<int> q;
	vector<bool> vizitate(nrNoduri+1, false);		//vector de bool initializat cu false, in el vom retine daca nodurile au fost vizitate
	vector<int> distante(nrNoduri+1, -1);			//vector de int care retine distantele de la nodul de start, initializat cu -1

	q.push(nodStart);

	vizitate[nodStart] = true;
	distante[nodStart] = 0;

	while (!q.empty())
	{
		int nodCurent = q.front();
		q.pop();

		for (int nodVecin : listaAdiacenta[nodCurent])
		{
			if (!vizitate[nodVecin])
			{
				vizitate[nodVecin] = true;			//BFS normal, iar in if() calculam si distanta dintre nodul curent si nodul de start
				q.push(nodVecin);					//Distanta creste cu 1 de fiecare data cand trecem de la un nod la altul
				distante[nodVecin] = distante[nodCurent] + 1;
			}
		}
	}

	return distante;
}

// Resume DFSAux:
//
// DFS care modifica lista de noduri vizitate din metoda NrCompConexeDFS
//

vector<bool> Graf::DFSAux(int nodStart, vector<bool> vizitate)
{
	vizitate[nodStart] = true;
	for (int i : listaAdiacenta[nodStart])
	{
		if (!vizitate[i])					
		{
			DFSAux(i, vizitate);
		}
	}
	
	return vizitate;
}

//
// Resume NrCompConexeDFS:
// 
// se determina numarul componentelor conexe ale grafului
// 
// Mod functionare:
// 
// Incrementam numarul de componente pentru fiecare apel al DFSAux
// Pt ca fiecare apel DFS marcheaza toate nodurile din cate o componenta conexa
// Deci fiecare apel ne baga intr-o componenta conexa noua
//

int Graf::NrCompConexeDFS(int nodStart)
{
	vector<bool> vizitate(nrNoduri+1, false);
	int nrComponente = 0;
	
	for (int i = 1; i < vizitate.size(); i++)
	{
		if (!vizitate[i])
		{									
			vizitate = DFSAux(i, vizitate);			
			nrComponente++;					
		}
	}
	return nrComponente;
}

//
// Resume BCXAux :
// 
// Aceasta metoda construieste vectorul primit ca parametru componenteBiconexe
// 
// Mod functionare:
// 
// Folosim algoritmul lui Tarjan, care construieste vectorii adancimi si adancimiMinime
// adancimi reprezinta adancimea fiecarui nod iar adancimi minime este adancimea minima la care se poate
// ajunge de la fiecare nod prin drumuri de intoarcere. Dupa ce construim acesti vectori prin apeluri DFS,
// identificam punctele critice apoi construim componenta cu toate nodurile care sunt retinute in stiva
// 

void Graf::BCXAux(int nodStart, int& adancimeStart, vector<int>& adancimi, vector<int>& adancimiMinime, vector<int>& parinti, stack<int>& s, vector<vector<int>>& componenteBCX)
{
	adancimi[nodStart] = adancimeStart;					//vector care retine adancimea nodurilor in parcurgerea DFS
	adancimiMinime[nodStart] = adancimeStart;			//vector care retine cat de sus(adancime) poate un nod sa ajunga pe un drum in "jos"
	adancimeStart++;									//adancimea curenta in DFS

	for (auto vecin : listaAdiacenta[nodStart])
	{
		s.push(nodStart);								//adaugam fiecare nod in stiva
		
		if (parinti[vecin] == -1)						//=> vecin nu este vizitat
		{
			parinti[vecin] = nodStart;					//parintele vecinului va fi nodul cu care apelam functia

			BCXAux(vecin, adancimeStart, adancimi, adancimiMinime, parinti, s, componenteBCX);		//DFS

			adancimiMinime[nodStart] = min(adancimiMinime[vecin], adancimiMinime[nodStart]);		//actualizam adancimea minima a nodului curent
																									//practic verificam daca fiul sau poate ajunge la un stramos al lui
																																											
				
			if (adancimi[nodStart] <= adancimiMinime[vecin])			//nodStart == punct critic    =>    aici se termina componenta biconexa
			{
				vector<int> componenta;
				componenta.push_back(nodStart);

				vector<bool> adaugat(nrNoduri + 1, false);				//nevoie de acest vector pt ca pusham acelasi nod de mai multe ori in stiva in unele cazuri
				adaugat[nodStart] = true;								

				int nod = s.top();
				s.pop();
				componenta.push_back(nod);
				adaugat[nod] = true;

				while (nod != nodStart)						//construim componenta cu toate nodurile din stiva 
				{												
					nod = s.top();					
					s.pop();
					if (!adaugat[nod]) {
						componenta.push_back(nod);
						adaugat[nod] = true;
					}	
				}

				componenteBCX.push_back(componenta);
			}
		}
		else
		{
			adancimiMinime[nodStart] = min(adancimi[vecin], adancimiMinime[nodStart]);				//daca vecin este vizitat => adancimi[vecin] < adancimi[nodStart]
		}																							//=> actualizam adancimeaMinima[nodStart] cu minimul dintre adancimea 
																									//vecinului si adancimeaMinima a nodStart
		
	}
}

//
//	Resume Biconexe:
// 
//	Construim vectorii necesari pe care ii trimitem ca si parametri in BCXAux,
// apoi afisam componentele biconexe
//

vector<vector<int>> Graf::Biconexe()
{
	vector<int> adancimi(nrNoduri + 1, -1);
	vector<int> adancimeMinima(nrNoduri + 1, -1);
	stack<int> s;
	vector<vector<int>> componente;
	vector<int> parinti(nrNoduri + 1, -1);
	int adancime = 1;
	parinti[1] = 1;
	BCXAux(1,adancime,adancimi,adancimeMinima,parinti,s, componente);

	return componente;
}

void countSort(vector<int>& v)
{
	int max = v[0];

	for (int i = 1; i < v.size(); i++)
	{
		if (v[i] > max)
		{
			max = v[i];
		}
	}

	vector<int> frecventa(max + 1, 0);
	int k = 0;

	for (int i = 0; i < v.size(); i++)
	{
		frecventa[v[i]]++;
	}

	for (int i = 0; i <= max; i++)
	{
		for (int j = 0; j < frecventa[i]; j++)
		{
			v[k++] = i;
		}

	}
};

//
// Resume HavelHakimi:
// 
// Verificam daca o secventa de int-uri care reprezinta gradele unor noduri
// poate forma un graf sau nu
//

bool Graf::HavelHakimi()
{	
	int n, nod;
	vector<int> secventa;
	cin >> n;
	for (int i = 0; i < n; i++)
	{
		cin >> nod;
		secventa.push_back(nod);
	}
	while (true) {
		
		countSort(secventa);

		int deScazut = secventa[0];

		secventa.erase(secventa.begin());
		
		if (secventa.size() < deScazut)
		{
			cout << "Nu merge";
			break;
		}

		for (int i = 0; i < deScazut; i++)
		{
			secventa[i]--;
		}

		bool merge = true;
		bool nuMerge = false;

		for (int i = 0; i < secventa.size(); i++)
		{
			if (secventa[i] != 0)
			{
				merge = false;
			}

			if (secventa[i] < 0)
			{
				nuMerge = true;
				break;
			}
		}

		if (merge)
		{
			return merge;
		}

		if (nuMerge)
		{
			return nuMerge;
		}
	}
}

//
// Resume CTCAux :
// 
// Aceasta metoda construieste vectorul primit ca parametru componenteCTC
// 
// Mod functionare:
// 
// Folosim algoritmul lui Tarjan, care construieste vectorii adancimi si adancimiMinime
// adancimi reprezinta adancimea fiecarui nod iar adancimi minime este adancimea minima la care se poate
// ajunge de la fiecare nod prin drumuri de intoarcere. Dupa ce construim acesti vectori prin apeluri DFS,
// identificam varfurile subarborilor care formeaza componente tare conexe 
// apoi construim componenta cu toate nodurile care sunt retinute in stiva
// 

void Graf::CTCAux(int nodStart, int& adancimeStart, vector<int>& adancimi, vector<int>& adancimeMinima, vector<bool>& peStiva, stack<int>& s, vector<vector<int>>& componenteCTC)
{
	adancimi[nodStart] = adancimeStart;					//vector care retine adancimea nodurilor in parcurgerea DFS
	adancimeMinima[nodStart] = adancimeStart;			//vector care retine cat de sus(adancime) poate un nod sa ajunga pe un drum in "jos"
	adancimeStart++;									//adancimea curenta in DFS

	s.push(nodStart);								//pusham fiecare nod in stiva

	peStiva[nodStart] = true;

	for (auto vecin : listaAdiacenta[nodStart])
	{
		if (adancimi[vecin] == -1)						//=> vecin nu este vizitat
		{
			CTCAux(vecin, adancimeStart, adancimi, adancimeMinima, peStiva, s, componenteCTC);		//DFS

			adancimeMinima[nodStart] = min(adancimeMinima[vecin], adancimeMinima[nodStart]);		//actualizam adancimea minima a nodului curent
																									//practic verificam daca fiul sau poate ajunge la un stramos al lui
		}
		else if(peStiva[vecin] == true)
		{
			adancimeMinima[nodStart] = min(adancimi[vecin], adancimeMinima[nodStart]);				//daca vecin este vizitat => adancimi[vecin] < adancimi[nodStart]
		}																							//=> actualizam adancimeaMinima[nodStart] cu minimul dintre adancimea 
																									//vecinului si adancimeaMinima a nodStart
	}

	int nod;

	if (adancimeMinima[nodStart] == adancimi[nodStart])							//varful subarborelui care formeaza CTC
	{
		vector<int>componenta;
		while (s.top() != nodStart)
		{
			nod = s.top();
			componenta.push_back(nod);						//construim componenta si o pusham in vectorul de componente
			peStiva[nod] = false;
			s.pop();
		}
		nod = s.top();
		componenta.push_back(nod);
		componenteCTC.push_back(componenta);
		peStiva[nod] = false;
		s.pop();

	}
}

//
//	Resume CTC:
// 
//	Construim vectorii necesari pe care ii trimitem ca si parametri in CTCAux,
// apoi afisam componentele tare conexe
//

vector<vector<int>> Graf::CTC()
{
	vector<int> adancimi(nrNoduri + 1, -1);
	vector<int> adancimeMinima(nrNoduri + 1, -1);
	stack<int> s;
	vector<vector<int>> componente;
	vector<bool> peStiva(nrNoduri + 1, false);
	int adancime = 1;
	for (int i = 1; i < nrNoduri + 1; i++)
	{
		if (adancimi[i] == -1)
		{
			CTCAux(i, adancime, adancimi, adancimeMinima, peStiva, s, componente);
		}
	}

	return componente;
}

//
// Resume MuchiiAux :
// 
// Aceasta metoda construieste vectorul primit ca parametru componenteCTC
// 
// Mod functionare:
// 
// Folosim algoritmul lui Tarjan, care construieste vectorii adancimi si adancimiMinime
// adancimi reprezinta adancimea fiecarui nod iar adancimi minime este adancimea minima la care se poate
// ajunge de la fiecare nod prin drumuri de intoarcere. Dupa ce construim acesti vectori prin apeluri DFS,
// identificam muchiile critice apoi construim vectorul care retine muchiile
// 

void Graf::MuchiiAux(int nodStart, int& adancimeStart, vector<int>& adancimi, vector<int>& adancimeMinima, vector<int>& parinti, vector<bool>& vizitate, vector<vector<int>>& MuchiiCritice)
{
	adancimi[nodStart] = adancimeStart;					//vector care retine adancimea nodurilor in parcurgerea DFS
	adancimeMinima[nodStart] = adancimeStart;			//vector care retine cat de sus(adancime) poate un nod sa ajunga pe un drum in "jos"
	adancimeStart++;									//adancimea curenta in DFS
	vizitate[nodStart] = true;
	for (auto vecin : listaAdiacenta[nodStart])
	{
		if (!vizitate[vecin])						//=> vecin nu este vizitat
		{

			parinti[vecin] = nodStart;					//parintele vecinului va fi nodul cuc are apelam functia

			MuchiiAux(vecin, adancimeStart, adancimi, adancimeMinima, parinti, vizitate, MuchiiCritice);		//DFS

			adancimeMinima[nodStart] = min(adancimeMinima[vecin], adancimeMinima[nodStart]);		//actualizam adancimea minima a nodului curent
																									//practic verificam daca fiul sau poate ajunge la un stramos al lui
			
			
			if (adancimeMinima[vecin] > adancimi[nodStart])				//conditie muchie critica
			{
				vector<int> muchieCritica;
				muchieCritica.push_back(nodStart);
				muchieCritica.push_back(vecin);

				MuchiiCritice.push_back(muchieCritica);
			}

		}
			
		else if(vecin != parinti[nodStart])				
		{
			adancimeMinima[nodStart] = min(adancimi[vecin], adancimeMinima[nodStart]);				//daca vecin este vizitat => adancimi[vecin] < adancimi[nodStart]
		}																							//=> actualizam adancimeaMinima[nodStart] cu minimul dintre adancimea 
																									//vecinului si adancimeaMinima a nodStart

	}
}

//
//	Resume MuchiiCrit():
// 
//	Construim vectorii necesari pe care ii trimitem ca si parametri in MuchiiAux,
// apoi afisam muchiile critice
//

vector<vector<int>> Graf::MuchiiCrit()
{
	vector<bool> vizitate(nrNoduri + 1, false);
	vector<int> adancimi(nrNoduri + 1, -1);
	vector<int> adancimeMinima(nrNoduri + 1, -1);
	vector<vector<int>> muchii;
	vector<int> parinti(nrNoduri + 1, -1);
	int adancime = 1;
	parinti[1] = 1;
	for (int i = 1; i < vizitate.size(); i++) 
	{
		if (!vizitate[i])
		{
			MuchiiAux(i, adancime, adancimi, adancimeMinima, parinti,vizitate, muchii);
		}
	}
	
	return muchii;

}

//
// Resume DFSTopological:
// 
// DFS care construieste stackul cu nodurile in ordine pentru sortarea topologica
// poate forma un graf sau nu
//

void Graf::DFSTopological(int nodStart, vector<bool>& vizitate, stack<int>& s)
{
	vizitate[nodStart] = true;
	for (int i : listaAdiacenta[nodStart])
	{
		if (!vizitate[i])
		{
			DFSTopological(i, vizitate, s);
		}
	}
	s.push(nodStart);
}

stack<int> Graf::SortareTopologica()
{
	stack<int> s;
	vector<bool> vizitate(nrNoduri + 1, false);

	for (int i = 1; i < vizitate.size(); i++)
	{
		if (!vizitate[i])
		{
			DFSTopological(i, vizitate, s);
		}
	}

	return s;
}

//---------------------------COD TEMA 2---------------------------------------------------------------------------------------------------

vector<int> costuri;		//Folosit pentru functia de comparare pentru sort-ul din Kruskall

class GrafPonderat :public Graf
{
protected:
	vector<pair<pair<int, int>, int>> listaMuchiiPonderate;		//Lista de muchii
	vector<vector<pair<int, int>>> listaAdiacentaPonderata;
	vector<vector<int>> matricePonderata;
private:
	void Reuniune(vector<int>&, int, int);			//Metoda Auxiliara pentru Kruskall + PaduriMultimiDisjuncte
	int FindMultime(vector<int>&, int);				//Metoda Auxiliara pentru Kruskall + PaduriMultimiDisjuncte
	static bool Compare(int m1, int m2)				//Metoda Auxiliara pentru Kruskall + PaduriMultimiDisjuncte
	{
		return costuri[m1] < costuri[m2];
	};
public:
	GrafPonderat();
	GrafPonderat(int, int, bool);

	void setMultiGraf();
	void setMatricePonderata();
	void setListaAdiacentaPonderata();
	void setlistaMuchiiPonderate();
	void Kruskall();								//Algoritmul lui Kruskall
	void PaduriMultimiDisjuncte();					//Creeaza multimi si ne spune daca doua elemente se afla in aceeasi mulime/nu
	vector<int> Dijkstra(int);						//Algoritmul lui Dijkstra
	vector<int> BellmanFord(int);					//Algoritmul lui Bellman Ford
	vector<vector<int>> RoyFloyd();					// Algoritmul Roy-Floyd
	void printMatriceCosturiMinime();				
	bool isEulerian();
	vector<int> CicluEulerian();
};

GrafPonderat::GrafPonderat() :Graf() {};

GrafPonderat::GrafPonderat(int nrN, int nrM, bool orr) :Graf(nrN, nrM, orr) {};

void GrafPonderat::setListaAdiacentaPonderata()
{
	int x, y, z;
	listaAdiacentaPonderata.resize(nrMuchii + 1);
	for (int i = 0; i < nrMuchii; i++)
	{
		fin >> x >> y >> z;
		listaAdiacentaPonderata[x].push_back(make_pair(y, z));
	}

}

void GrafPonderat::setlistaMuchiiPonderate()
{
	int x, y, z;

	for (int i = 0; i < nrMuchii; i++)
	{
		fin >> x >> y >> z;
		listaMuchiiPonderate.push_back(make_pair(make_pair(x, y), z));
	}
}

//
// Resume BellmanFord():
// 
// Determinam daca in graful dat exista un ciclu de cost negativ.
// Daca nu exista, se determina costul minim al unui lant de la nodul 1 la fiecare dintre nodurile 2, 3, ... , N-1, N.
// 
// Mod de functionare:
// 
// Initializam distantele de la nodSursa la toate celelalte noduri cu o valoare cat mai mare
// Declaram o coada in care vom retine nodurile si un vector in care retinem daca un nod formeaza un ciclu negativ sau nu.
// 
// Cat timp mai avem elemente in coada (nodSursa este adaugat initial in coada), luam primul nod din ea
// si parcurgem toate nodurile adiacente cu acesta. Daca muchia formata din cele doua noduri poate fii relaxata,
// o relaxam, si daca acesta nu se afla in coada il adaugam in ea. Valoarea nodului din formCiclu este incrementata, daca
// aceasta depaseste nrNoduri+1 atunci formeaza un ciclu negativ.
//

vector<int> GrafPonderat::BellmanFord(int nodSursa)
{
	vector<int> distante(nrNoduri + 1, 2000000);
	distante[nodSursa] = 0;

	queue<int> q;
	q.push(nodSursa);

	vector<bool> inQueue(nrNoduri + 1, false);
	inQueue[nodSursa] = true;

	vector<int> formCiclu(nrNoduri + 1, 0);

	while (!q.empty())
	{
		int nodCurent = q.front();
		q.pop();
		inQueue[nodCurent] = false;

		for (auto muchie : listaAdiacentaPonderata[nodCurent])
		{
			int nodAdiacent = muchie.first;
			int greutateMuchie = muchie.second;

			if (distante[nodCurent] != 2000000 && distante[nodAdiacent] > distante[nodCurent] + greutateMuchie)
			{
				distante[nodAdiacent] = distante[nodCurent] + greutateMuchie;

				if (!inQueue[nodAdiacent])
				{
					q.push(nodAdiacent);
					inQueue[nodAdiacent] = true;
					formCiclu[nodAdiacent]++;

					if (formCiclu[nodAdiacent] > nrNoduri + 1)
					{
						fout << "Ciclu negativ!";
						return;
					}
				}
			}
		}
	}

	for (int i = 2; i <= nrNoduri; i++)
	{
		if (distante[i] == 2000000)
		{
			distante[i] = 0;
		}
	}

	return distante;

}

//
// Resume Dijkstra():
// 
// Se determina lungimea minima a drumului de la nodul 1 la fiecare din nodurile 2, 3, ..., N-1, N 
// si sa se afiseze aceste lungimi. Lungimea unui drum este data de suma lungimilor arcelor care constituie drumul.
// 
// Mod de functionare:
// 
// Declaram un Avl de pair-uri si inseram in el primul nod, cu greutatea 0. Vom tine greutatea ca primul
// element din pair, ca sa avem nodul cu greutatea minim ain varful Avlului.
// Cat timp avem elemente in Avl, adica mai avem distante minime de verificat, extragem nodul din Avl,
// iteram prin adiacentii lui, si verificam daca muchia formata trebuie relaxata. Daca da o relaxam si apoi daca muchia
// a mai fost relaxata stergem nodul din Avl si il reintroducem cu noua distanta minima
// Apoi afisam distantele.
// 

vector<int> GrafPonderat::Dijkstra(int nodSursa)
{
	set<pair<int, int>> Avl;
	Avl.insert({ 0, nodSursa });

	vector<int> distante(nrNoduri + 1, 2000000);
	distante[nodSursa] = 0;

	while (!Avl.empty())
	{
		pair<int, int> nodCurent = *Avl.begin();
		Avl.erase(Avl.begin());

		for (auto nodAdiacent : listaAdiacentaPonderata[nodCurent.second])
		{
			if (distante[nodAdiacent.first] > distante[nodCurent.second] + nodAdiacent.second)
			{
				if (distante[nodAdiacent.first] != 2000000)
				{
					Avl.erase(Avl.find(make_pair(distante[nodAdiacent.first], nodAdiacent.first)));
				}
				distante[nodAdiacent.first] = distante[nodCurent.second] + nodAdiacent.second;
				Avl.insert(make_pair(distante[nodAdiacent.first], nodAdiacent.first));
			}
		}
	}

	for (int i = 2; i <= nrNoduri; i++)
	{
		if (distante[i] == 2000000)
		{
			distante[i] = 0;
		}
	}

	return distante;
}

//
// Resume PaduriMultimiDisjuncte:
// 
// Aplicam metodele folosite in algoritmul lui Dijksrtra
//

void GrafPonderat::PaduriMultimiDisjuncte()
{
	int n, m;
	fin >> n >> m;
	vector<int> multime(n + 1, -1);
	vector<int> indici;
	for (int i = 1; i < n + 1; i++)
	{
		multime[i] = i;
	}
	for (int i = 1; i <= m; i++)
	{

		int operatie;
		fin >> operatie;
		if (operatie == 1)
		{
			int x, y;
			fin >> x >> y;
			Reuniune(multime, x, y);
		}

		else
		{
			int x, y;
			fin >> x >> y;

			if (FindMultime(multime, x) == FindMultime(multime, y))
			{
				fout << "DA\n";
			}
			else
			{
				fout << "NU\n";
			}
		}
	}
}

//
// Resume Reuniune():
// 
// adauga nodul i in multimea lui j
//

void GrafPonderat::Reuniune(vector<int>& multime, int i, int j)
{
	multime[FindMultime(multime, i)] = FindMultime(multime, j);
}

int GrafPonderat::FindMultime(vector<int>& multime, int i)
{
	if (multime[i] == i)
	{
		return i;
	}
	multime[i] = FindMultime(multime, multime[i]);
	return multime[i];
}

//
// Resume Kruskall():
// 
// Afiseaza un arbore partial de cost minim al grafului
// 
// Mod de functionare:
// 
// Aplica algoritmul lui Kruskall. Sorteaza muchiile dupa costuri crescator
// apoi pt fiecare muchie verifica daca nodurile care formeaza muchia sunt in aceeasi componenta conexa
// cu ajutorul multimilor disjuncte. daca nu sunt in aceeasi multime folosim functia de reuniune
// si apoi adaugam muchia in vectorul in care retinem muchii
//

void GrafPonderat::Kruskall()
{
	vector<int> indici(nrMuchii, -1);
	vector<int> multime(nrNoduri + 1, -1);
	int suma = 0;
	vector<int> muchii;

	for (int i = 0; i < nrMuchii; i++)
	{
		indici[i] = i;
	}

	for (int i = 1; i <= nrNoduri; i++)
	{
		multime[i] = i;
	}

	for (auto i : listaMuchiiPonderate)
	{
		costuri.push_back(i.second);
	}

	sort(indici.begin(), indici.end(), Compare);

	for (int i = 1; i <= nrMuchii; i++)
	{
		int indexIndici = i - 1;
		if (FindMultime(multime, listaMuchiiPonderate[indici[indexIndici]].first.first) != FindMultime(multime, listaMuchiiPonderate[indici[indexIndici]].first.second))
		{
			suma += listaMuchiiPonderate[indici[indexIndici]].second;
			Reuniune(multime, listaMuchiiPonderate[indici[indexIndici]].first.first, listaMuchiiPonderate[indici[indexIndici]].first.second);
			muchii.push_back(indici[indexIndici]);
		}
	}

	fout << suma << "\n";
	fout << muchii.size() << "\n";
	for (int i = 0; i < muchii.size(); i++)
	{
		fout << listaMuchiiPonderate[muchii[i]].first.first << ' ' << listaMuchiiPonderate[muchii[i]].first.second << "\n";
	}
}

//---------------------------COD TEMA 3---------------------------------------------------------------------------------------------------

void GrafPonderat::printMatriceCosturiMinime()
{
	vector<vector<int>> sol;

	sol = RoyFloyd();

	for (auto i : sol)
	{
		for (auto j : i)
		{
			fout << j << ' ';
		}
		fout << "\n";
	}
}

//
// Resume RoyFloyd():
// 
// Returneaza matricea costurilor minime, care reprezinta costurile minime de la
// oricare nod la altul. Daca nu se poate ajunge de la un anumit nod la altul, costul acestei perechi va fii 0
// 
// Mod de functionare:
// 
// Algoritmul compara toate caile posibile prin graf intre fiecare pereche de noduri.
// Luam pe rand fiecare nod si il consideram un nod intermediar(adica drumul cu cost minim trece prin el).
// Daca facem asta inseamna ca deja am considerat nodurile 1..k-1 ca fiind deja intermediare.
// In cazul asta avem doua situatii: K este nod intermediar si updatam distanta, sau nu este si trecem mai departe.
// 

vector<vector<int>> GrafPonderat::RoyFloyd()
{
	vector<vector<int>>matriceCosturi;

	matriceCosturi.resize(matricePonderata.size());
	for (int i = 0; i < matricePonderata.size(); i++)
	{
		matriceCosturi[i].resize(matricePonderata[i].size());
		for (int j = 0; j < matricePonderata.size(); j++)
		{
			matriceCosturi[i][j] = matricePonderata[i][j] ;
		}
	}

	for (int k = 0; k < matricePonderata.size(); k++)
	{
		for (int i = 0; i < matricePonderata.size(); i++)
		{
			for (int j = 0; j < matricePonderata.size(); j++)
			{
				if ((matriceCosturi[i][j] > (matriceCosturi[i][k] + matriceCosturi[k][j]) || matriceCosturi[i][j] == 0) && (matriceCosturi[i][k] != 0 && matriceCosturi[k][j] != 0) && (i!=j))
				{
					matriceCosturi[i][j] = matriceCosturi[i][k] + matriceCosturi[k][j];
				}
			}
		}
	}

	return matriceCosturi;

}

void GrafPonderat::setMatricePonderata()
{
	int n;
	fin >> n;
	matricePonderata.resize(n);
	for (int i = 0; i < n; i++)
	{
		matricePonderata[i].resize(n);
		for (int j = 0; j < n; j++)
		{
			int cost;
			fin >> cost;
			matricePonderata[i][j] = cost;
		}
	}
}

//
// Resume DiametruArbore():
// 
// Aflam lungimea drumului intre cele mai departate doua frunze.
// 
// Mod de functionare:
// 
// Facem un BFS din primul nod al grafului (merge din orice nod). Acesta ne va ajuta sa gasim
// cea mai indepartata frunza de primul nod. Apoi facem un al doilea BFS pornind din aceasta frunza
// gasind astfel cel mai indepartat nod de aceasta frunza. Aceste doua noduri sunt cele mai indepartate din graf, asa ca
// aplicam algoritmul de distante, folosind un al treilea bfs, ca sa gasim distanta dintre cele 2 frunze.
// 

int Graf::DiametruArbore()
{
	int diametru = 0;
	int frunza1, frunza2;

	queue<int> q;
	vector<bool> vizitate(nrNoduri + 1, false);

	q.push(1);

	vizitate[1] = true;

	while (!q.empty())
	{
		int nodCurent = q.front();
		q.pop();

		for (int nodVecin : listaAdiacenta[nodCurent])
		{
			if (!vizitate[nodVecin])
			{
				vizitate[nodVecin] = true;			
				q.push(nodVecin);					
			}
			frunza1 = nodCurent;
		}
	}

	for (auto i : vizitate)
	{
		i = false;
	}

	q.push(frunza1);

	vizitate[frunza1] = true;

	while (!q.empty())
	{
		int nodCurent = q.front();
		q.pop();

		for (int nodVecin : listaAdiacenta[nodCurent])
		{
			if (!vizitate[nodVecin])
			{
				vizitate[nodVecin] = true;			
				q.push(nodVecin);	
			}
			frunza2 = nodCurent;
		}
	}

	for (auto i : vizitate)
	{
		i = false;
	}

	q.push(frunza1);
	vector<int> distante(nrNoduri + 1, -1);
	vizitate[frunza1] = true;
	distante[frunza1] = 0;

	while (!vizitate[frunza2])
	{
		int nodCurent = q.front();
		q.pop();

		for (int nodVecin : listaAdiacenta[nodCurent])
		{
			if (!vizitate[nodVecin])
			{
				vizitate[nodVecin] = true;
				q.push(nodVecin);
				distante[nodVecin] = distante[nodCurent] + 1;
			}
		}

		
	}
	diametru = distante[frunza2]+1;
	return diametru;
}

//---------------------------COD TEMA 4---------------------------------------------------------------------------------------------------

bool GrafPonderat::isEulerian()
{
	for (auto i : listaAdiacentaPonderata)
	{
		if (i.size() % 2 != 0)
		{
			return false;
		}
	}
	return true;
}

vector<int> GrafPonderat :: CicluEulerian()
{

}

void GrafPonderat::setMultiGraf()
{
	int x, y, z=1;
	listaAdiacentaPonderata.resize(nrMuchii + 1);
	for (int i = 0; i < nrMuchii; i++)
	{
		fin >> x >> y;
		listaAdiacentaPonderata[x].push_back(make_pair(y, z));
		z++;
	}
}

int main()
{

	return 0;
}
