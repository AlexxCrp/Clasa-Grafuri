#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>

using namespace std;

ifstream fin("numeFisier.in");
ofstream fout("numeFisier.out");

class Graf
{
private:

	int nrMuchii, nrNoduri;
	vector<vector<int>> listaAdiacenta;
	bool orientat;

	void adaugaMuchie(int, int, bool);		    //Am decis sa fac adaugarea unei muchii in lista de adiacenta o functie, poate o sa fie useful later?
	void DFSAux(int, vector<bool>&);			//Functie auxiliara pt problema DFS de pe infoarena
	void BCXAux(int, int&, vector<int>&, vector<int>&, vector<int>&, stack<int>&, vector<vector<int>>&);		//Metoda auxiliara pt problema Componente Biconexe de pe infoarena
	void CTCAux(int, int&, vector<int>&, vector<int>&, vector<bool>&, stack<int>&, vector<vector<int>>&);		//Metoda auxiliara pt CTC
	void MuchiiAux(int, int&, vector<int>&, vector<int>&, vector<int>&, vector<bool>&, vector<vector<int>>&);	//Metoda auxiliara pt MuchiiCrit()
	void DFSTopological(int, vector<bool>&, stack<int>&);			//Metoda auxiliara pt Sortarea topologica
public:
	Graf();
	Graf(int, int, bool);
	void BFS(int);							//Metoda care rezolva problema BFS de pe infoarena
	int DFSrezolvare(int);					//Metoda care rezolva problema DFS de pe infoarena
	void Biconexe();						//Metoda care rezolva problema Componente Biconexe de pe infoarena
	void HavelHakimi();						//Metoda care pune in practica Havel Hakimi	
	void CTC();								//Metoda care afiseaza componentele tare conexe ale grafului	
	void MuchiiCrit();						//Metoda care afiseaza muchiile critice ale grafului
	void SortareTopologica();				//Metoda care sorteaza topologic nodurile grafului
};

Graf::Graf()
{
	nrMuchii = 0;
	nrNoduri = 0;
	orientat = false;
}

Graf::Graf(int n, int m, bool orr)
{
	int x, y;

	nrNoduri = n;
	nrMuchii = m;
	orientat = orr;
	listaAdiacenta.resize(nrNoduri + 1);	//Construim si reprezentarea grafului in constructor

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

void Graf::BFS(int nodStart)
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

	for (int i=1; i < distante.size(); i++)
	{
		fout << distante[i] << ' ';					//Afisam distantele
	}
}

void Graf::DFSAux(int nodStart, vector<bool>& vizitate)
{
	vizitate[nodStart] = true;
	for (int i : listaAdiacenta[nodStart])
	{
		if (!vizitate[i])					//DFS care modifica lista de noduri vizitate din metoda care rezolva problema DFS
		{
			DFSAux(i, vizitate);
		}
	}
	
}

int Graf::DFSrezolvare(int nodStart)
{
	vector<bool> vizitate(nrNoduri+1, false);
	int nrComponente = 0;
	
	for (int i = 1; i < vizitate.size(); i++)
	{
		if (!vizitate[i])
		{									//Incrementam numarul de componente pentru fiecare apel al DFSAux
			DFSAux(i, vizitate);			//Pt ca fiecare apel DFS marcheaza toate nodurile din cate o componenta conexa
			nrComponente++;					//Deci fiecare apel ne baga intr-o componenta conexa noua
		}
	}
	return nrComponente;
}

void Graf::DFSTopological(int nodStart, vector<bool>& vizitate, stack<int>& s)
{
	vizitate[nodStart] = true;
	for (int i : listaAdiacenta[nodStart])
	{
		if (!vizitate[i])
		{
			DFSTopological(i, vizitate, s);				//DFS care construieste stackul cu nodurile in ordine pentru sortarea topologica
		}												//precum ne spun si indicatiile de rezolvare de pe infoarena
	}
	s.push(nodStart);
}

void Graf::BCXAux(int nodStart, int& adancimeStart, vector<int>& adancimi, vector<int>& adancimeMinima, vector<int>& parinti, stack<int>& s, vector<vector<int>>& componenteBCX)
{
	adancimi[nodStart] = adancimeStart;					//vector care retine adancimea nodurilor in parcurgerea DFS
	adancimeMinima[nodStart] = adancimeStart;			//vector care retine cat de sus(adancime) poate un nod sa ajunga pe un drum in "jos"
	adancimeStart++;									//adancimea curenta in DFS

	for (auto vecin : listaAdiacenta[nodStart])
	{
		s.push(nodStart);								//pusham fiecare nod in stiva
		
		if (parinti[vecin] == -1)						//=> vecin nu este vizitat
		{
			parinti[vecin] = nodStart;					//parintele vecinului va fi nodul cuc are apelam functia

			BCXAux(vecin, adancimeStart, adancimi, adancimeMinima, parinti, s, componenteBCX);		//DFS

			adancimeMinima[nodStart] = min(adancimeMinima[vecin], adancimeMinima[nodStart]);		//actualizam adancimea minima a nodului curent
																									//practic verificam daca fiul sau poate ajunge la un stramos al lui
																																											
				
			if (adancimi[nodStart] <= adancimeMinima[vecin])			//nodStart == punct critic    =>    aici se termina componenta biconexa
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
			adancimeMinima[nodStart] = min(adancimi[vecin], adancimeMinima[nodStart]);				//daca vecin este vizitat => adancimi[vecin] < adancimi[nodStart]
		}																							//=> actualizam adancimeaMinima[nodStart] cu minimul dintre adancimea 
																									//vecinului si adancimeaMinima a nodStart
		
	}
}

void Graf::Biconexe()
{
	vector<int> adancimi(nrNoduri + 1, -1);
	vector<int> adancimeMinima(nrNoduri + 1, -1);
	stack<int> s;
	vector<vector<int>> componente;
	vector<int> parinti(nrNoduri + 1, -1);
	int adancime = 1;
	parinti[1] = 1;
	BCXAux(1,adancime,adancimi,adancimeMinima,parinti,s, componente);

	fout << componente.size() << "\n";
	for (auto i : componente)
	{
		for (auto j : i)
		{
			fout << j << ' ';
		}
		fout << "\n";
	}

	
}

void Graf::HavelHakimi()
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
		sort(secventa.begin(), secventa.end());
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
			cout << "merge";
			break;
		}

		if (nuMerge)
		{
			cout << "nu merge";
			break;
		}
	}
	
}

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

void Graf::CTC()
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
	fout << componente.size() << "\n";
	for (auto i : componente)
	{
		for (auto j : i)
		{
			fout << j << ' ';
		}
		fout << "\n";
	}
}

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

void Graf::MuchiiCrit()
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
	for (auto i : muchii)
	{
		for (auto j : i)
		{
			fout << j << ' ';
		}
		fout << "\n";
	}


}

void Graf::SortareTopologica()
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

	while (!s.empty())
	{
		fout << s.top()<<' ';
		s.pop();
	}
}

int main()
{
	int n, m;
	fin >> n >> m;
	//Graf G(n, m, true/false);
	
	//G.Metoda();


	//Acest proiect nu ar fi fost posibil fara geeksforgeeks.org, asa ca vreau sa le transmit multumiri :))

	return 0;
}