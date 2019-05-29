#include <stdio.h>
#include <stdlib.h>
#include <string.h>     
#include <time.h>
#include <ncurses.h>        //libreria per le frecce

//in base al sistema operativo per la funzione sleep_ms
#ifdef WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

//definisco numero delle navi  
#define n_navi 10

// nella vita A=100% B=66.6% C= 50% D= 33.3% E=abbattuta F=75% G=25% 
// nella lunghezza della nave A=1 B=2 C=3 D=4

int lunghezza = 10;     //variabile lunghezza campo
int G_conta_tasti = 0, x_tab=0, y_tab=0;     //variabili per tasti
char alf[26]={'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};     //array alfabeto per creazione schema di gioco
// 3 tabelle 
int lett_us=0;      //lettere usate per navi + dimensioni
int navi_aff_1=0, navi_aff_2=0;
char tabella[50][50][5];   //tabella player    
char tabella_pl2[50][50];   //tabella player 
char tabella_spari[50][50];     //tabella per decidere dove colpire
char tabella_spari_pl2[50][50];     //tabella per decidere dove colpire
char tabella_bot[50][50];       //tabella del bot
int G_nave_att=0;
bool single=false;
//prototipi funzione
void riempi(void);              // funzione serve ad allineare per l'inizio e riempire tabelle con ' ' 
void a_navi(int scelta);              //funzione per aggiunta nave
void display(int scelta);   //funzione per visualizzare campo di gioco
void scelta_cas(void);      //scelta casella dove colpire
void verifica(int riga, int colonna, int valore);       //verifica del colpo (nave colpita o no)
void emit_beep(void);   //funzione per beep sonoro
void frase(char frase[50]);     //funzione per scrivere una prase con animazione
void sleep_ms(int milliseconds);        //funzione per attendere millisecondi
void tasti(int scelta);         //funzione per gestione frecce e scelte
int menu(void);
void scelta_bot(void); 
int finita (int val);
int main() {    
    //-----------------------------------------------------------------
    //funzioni per inizializzare ncurses    
    initscr();      
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    scrollok(stdscr, TRUE);
    //-----------------------------------------------------------------

    //display(0);     //richiamo funzione per mostrare il campo passando come parametro 0 --> mostra campo player
    srand(time(NULL));          //genero seed per numeri pseudo-casuali
    int val=menu();
    int fine=0, ripeti=1;
    
    if(val==0){
        single=true;
        
        //singleplayer
        while(ripeti){
            riempi();       //richiamo funzione iniziale per allineare il campo

            navi_aff_1=0;
            navi_aff_2=0; 
            display(0);     //richiamo funzione per mostrare il campo passando come parametro 0 --> mostra campo player
            a_navi(0);       //funzione per posizionamento navi

            erase();        //erase è la funzione di ncurses che sostitusce system(clear)
            refresh();      //refresh è una funzione di ncurses per il refresh del terminale
            
            while(navi_aff_1<10&&navi_aff_2<10){
                erase();
                display(0);
                sleep_ms(3500);
                frase("Turno del player.");
                sleep_ms(500);
                scelta_cas();       //funzione per far scegliere all'utente dove colpire
                frase("Turno del bot.");
                sleep_ms(500);
                scelta_bot();       //bot tenta di colpire la nave
                
            }
            if(navi_aff_1>9){
                fine=0;
            }else{
                fine=1;
            }
            if(finita(fine)){
                ripeti=0;
                break;
            }else{
                ripeti=1;
            }
        }
       

       
    }else{
        single=false;
        riempi();       //richiamo funzione iniziale per allineare il campo

        display(0); 
        a_navi(0); 
        erase();
        frase("Turno del secondo player.");
        display(3);
        a_navi(1);

    }
    /*a_navi();       //funzione per posizionamento navi

    erase();        //erase è la funzione di ncurses che sostitusce system(clear)
    refresh();      //refresh è una funzione di ncurses per il refresh del terminale
    
   
    scelta_cas();       //funzione per far scegliere all'utente dove colpire
    */
    system("stty sane");        //resetto il terminale
    return 0;
}
void riempi(){
    /*
         _______________________________________________________________________
        |                                                                       |
        |   LEGENDA FUNZIONE                                                    |
        |        - NESSUN PARAMETRO IN INPUT                                    |
        |        - ESEGUE ALLINEAMENTO TABELLE E POSIZIONE DELLE NAVI NEMICHE   |
        |                                                                       |
        |_______________________________________________________________________|

    */        
    int i=0, j=0;       //dichiaro ed inizializzo variabili contatore
    for (i=0; i<lunghezza; i++) {       //for che cicla per la lunghezza della tabella
        for (j=0; j<lunghezza; j++) {       //for che cicla per la lunghessa della tabella
            tabella[i][j][0]=' ';              //assegno carattere spazio alla tabella del player
            tabella_bot[i][j]=' ';          //assegno carattere spazio alla tabella del bot
            tabella_spari[i][j]=' ';        //"          " alla tabella degli spari
            tabella_pl2[i][j]=' ';
            tabella_spari_pl2[i][j]=' ';
        }
    }
    for (i=0; i<n_navi; i++) {      //for che cicla n_navi volte
        
        int riga = (rand() % (lunghezza-1 + 1));        //genero numero per riga
        int colonna = (rand() % (lunghezza-1 + 1));     //genero numero per colonna
        while (tabella_bot[riga][colonna]!=' ') {       //se la tabella_bot alle posizioni generate è occupata
            //se occupata genero nuovi numeri 
            riga = (rand() % (lunghezza-1 + 1));        
            colonna = (rand() % (lunghezza-1 + 1));
        }
        tabella_bot[riga][colonna]='X';     //inserisco la nave nella posizione generata
    }
}

void tasti(int scelta){
    /*
         _______________________________________________________________________
        |                                                                       |
        |   LEGENDA FUNZIONE                                                    |
        |        - PARAMETRO SCELTA IN INPUT                                    |
        |            - IN BASE A SCELTA VIENE DECISA LA TABELLA DA USARE        |
        |        - GESTISCE LA PRESSIONE DEI TASTI DA TASTIERA PER SCEGLIERE    |
        |           POSIZIONE NELLA TABELLA                                     |
        |                                                                       |
        |_______________________________________________________________________|

    */ 
    int ch;     //dichiaro ch che conterrà il carattere premuto
    int pos_1=0, pos_2=0, pos_3=0;
    int num;
    char lett='A';

    while (1) {     //while true
        
        ch = getch();          //ottengo tasto premuto

        
        refresh();
      
        if (ch==32){
            
            int i=0, j=0;
            for ( i = 0; i < lunghezza; i++)
            {
                for ( j = 0; j < lunghezza; j++)
                {
                    if(tabella[i][j][0]=='O'){
                        tabella[i][j][0]=' ';
                        
                    }
                   
                 }
            }
            display(0);
            
            
        }
        if (scelta==1){        //se la scelta è 1
            if(ch == KEY_RIGHT){        //controllo se il tasto premuto è la freccia destra
            
            x_tab+=1;       //aumento la variabile 'globale' della posizione in x del cursore 'O'
            if(x_tab>lunghezza-1){      //se la pos del cursore in fondo alla riga
                if(tabella_spari[y_tab][x_tab-1]=='O'){     //controllo se la tabella ha nella posizioni precedente il cursore
                    tabella_spari[y_tab][x_tab-1]= ' ';     //allora pulisco quella cella
                }
                
                x_tab=0;        //essendo in fondo alla linea, lo riporto all'inizio
                if(y_tab==lunghezza-1){    //controllo se il cursore è in fondo alla tabella
                    y_tab=0;        //essendo in fondo all'intera tabella lo riporto all'inizio 
                }else
                {
                    y_tab+=1;       //altrimenti lo porto una riga sotto
                }
                
                
                if(tabella_spari[y_tab][x_tab]!='X'){       //infine controllo che nella nuova posizione del cursore non ci sia gia una nave
                    tabella_spari[y_tab][x_tab]= 'O';       //posiziono il nuovo cursore
                }
                
            }else if(x_tab==0){         //altrimenti se si trova all'inizio di una riga
                
                if(tabella_spari[y_tab][x_tab]!='X'){       //controllo che non sia gia presente una nave
                    tabella_spari[y_tab][x_tab]= 'O';       //posizioni il cursore
                }
            }else{      //altimenti (il cursore è in una qualsiasi altra posizione)
                if(tabella_spari[y_tab][x_tab-1]=='O'){     //controllo se nella posizione precedente ci sia il cursore
                    tabella_spari[y_tab][x_tab-1]= ' ';          //pulisco la cella precedente
                }       
                
                if(tabella_spari[y_tab][x_tab]!='X'){       //controllo che nella nuova posizione del cursore non sia gia presente una nave 
                    tabella_spari[y_tab][x_tab]= 'O';           //posiziono il cursore
                }
            }
            
            display(1);     //mostro la tabella con parametro 1 perchè si tratta della tabella spari in questo caso
        }
        if(ch == KEY_LEFT){
            
            x_tab-=1;
            if(x_tab<0){
                if(tabella_spari[y_tab][x_tab+1]=='O'){
                    tabella_spari[y_tab][x_tab+1]= ' ';
                }
                
                x_tab=lunghezza-1;
                if(y_tab==0){
                    y_tab=lunghezza-1;
                }else{
                    y_tab-=1;
                }
                
                
                if(tabella_spari[y_tab][x_tab]!='X'){
                    tabella_spari[y_tab][x_tab]= 'O';
                }
            }else{
                if(tabella_spari[y_tab][x_tab+1]=='O'){
                    tabella_spari[y_tab][x_tab+1]= ' ';
                }
                if(tabella_spari[y_tab][x_tab]!='X'){
                    tabella_spari[y_tab][x_tab]= 'O';
                }
            }
            
            display(1);
        }
        if(ch == KEY_DOWN){
            
            y_tab+=1;
            if(y_tab>lunghezza-1){
                if(x_tab==lunghezza-1){
                    if(tabella_spari[y_tab-1][x_tab]=='O'){
                    tabella_spari[y_tab-1][x_tab]= ' ';
                }
                    y_tab=0;
                    x_tab=0;
                    
                    if(tabella_spari[y_tab][x_tab]!='X'){
                    tabella_spari[y_tab][x_tab]= 'O';
                }
                }else
                {
                    if(tabella_spari[y_tab-1][x_tab]=='O'){
                    tabella_spari[y_tab-1][x_tab]= ' ';
                }
                    x_tab+=1;
                    y_tab=0;
                    if(tabella_spari[y_tab][x_tab]!='X'){
                    tabella_spari[y_tab][x_tab]= 'O';
                }
                }
                
            }else{
                if(tabella_spari[y_tab-1][x_tab]=='O'){
                    tabella_spari[y_tab-1][x_tab]= ' ';
                }
                if(tabella_spari[y_tab][x_tab]!='X'){
                    tabella_spari[y_tab][x_tab]= 'O';
                }
            }
            
            display(1);
        }
        if(ch == KEY_UP){
            
            y_tab-=1;
            if(y_tab<0){
                if(x_tab==0){
                    if(tabella_spari[y_tab+1][x_tab]=='O'){
                    tabella_spari[y_tab+1][x_tab]= ' ';
                }
                    y_tab=lunghezza-1;
                    x_tab=lunghezza-1;
                    if(tabella_spari[y_tab][x_tab]!='X'){
                    tabella_spari[y_tab][x_tab]= 'O';
                }
                }else{
                    if(tabella_spari[y_tab+1][x_tab]=='O'){
                    tabella_spari[y_tab+1][x_tab]= ' ';
                }
                    y_tab=lunghezza-1;
                    x_tab-=1;
                    
                    if(tabella_spari[y_tab][x_tab]!='X'){
                    tabella_spari[y_tab][x_tab]= 'O';
                }
                }
                
            }else{
                if(tabella_spari[y_tab+1][x_tab]=='O'){
                    tabella_spari[y_tab+1][x_tab]= ' ';
                }
                if(tabella_spari[y_tab][x_tab]!='X'){
                    tabella_spari[y_tab][x_tab]= 'O';
                }
            }
            
            display(1);
        }
        if(ch == 10){
            
            
            tabella_spari[y_tab][x_tab]='X';
            display(1);
            break;
            
        }
        }else if(scelta==0){          //se la scelta è 0
            
            if(x_tab==0&&y_tab==0){
                pos_1=x_tab+1;
                pos_2=pos_1+1;
                pos_3=pos_2+1;
                tabella[y_tab][x_tab][0]='O';
                tabella[y_tab][pos_1][0]='O';
                tabella[y_tab][pos_2][0]='O';
                tabella[y_tab][pos_3][0]='O';
            }

            if(ch == KEY_RIGHT){
            if(G_nave_att==4){
                //devo posizionare nave lunga 4
                
                x_tab++;
                pos_1++;
                pos_2++;
                pos_3++;
                if(pos_3>lunghezza-1){
                    //out of index
                    if(y_tab==lunghezza-1){
                        y_tab=0;
                       
                        if(tabella[lunghezza-1][x_tab-1][0]=='O'){
                            tabella[lunghezza-1][x_tab-1][0]=' ';
                            
                        }
                        if (tabella[lunghezza-1][pos_1-1][0]=='O'){
                            tabella[lunghezza-1][pos_1-1][0]=' ';
                        }
                        if(tabella[lunghezza-1][pos_2-1][0]=='O'){
                            tabella[lunghezza-1][pos_2-1][0]=' ';
                        }
                        if(tabella[lunghezza-1][pos_3-1][0]=='O'){
                            tabella[lunghezza-1][pos_3-1][0]=' ';
                        }
                        
                    }else{
                        y_tab++;
                        
                        if(tabella[y_tab-1][x_tab-1][0]=='O'){
                            tabella[y_tab-1][x_tab-1][0]=' ';
                            
                        }
                        if (tabella[y_tab-1][pos_1-1][0]=='O'){
                            tabella[y_tab-1][pos_1-1][0]=' ';
                        }
                        if(tabella[y_tab-1][pos_2-1][0]=='O'){
                           tabella[y_tab-1][pos_2-1][0]=' ';
                        }
                        if(tabella[y_tab-1][pos_3-1][0]=='O'){
                            tabella[y_tab-1][pos_3-1][0]=' ';
                        }
                        

                    }
                    
                        
                    x_tab=0;
                    pos_1=1;
                    pos_2=2;
                    pos_3=3;

                }
                if(tabella[y_tab][x_tab-1][0]!='X'){
                    tabella[y_tab][x_tab-1][0]=' ';
                }
                if (tabella[y_tab][pos_1-1][0]=='O'){
                    tabella[y_tab][pos_1-1][0]=' ';
                 }
                if(tabella[y_tab][pos_2-1][0]=='O'){
                     tabella[y_tab][pos_2-1][0]=' ';
                }
                if(tabella[y_tab][pos_3-1][0]=='O'){
                    tabella[y_tab][pos_3-1][0]=' ';
                }
                
                if(tabella[y_tab][x_tab][0]!='X'&&tabella[y_tab][pos_1][0]!='X'&&tabella[y_tab][pos_2][0]!='X'&&tabella[y_tab][pos_3][0]!='X'){
                    tabella[y_tab][x_tab][0]='O';
                    tabella[y_tab][pos_1][0]='O';
                    tabella[y_tab][pos_2][0]='O';
                    tabella[y_tab][pos_3][0]='O';
                }
                
                display(0);
            }else if(G_nave_att==3){
                //devo posizionare nave lunga 3
                
                x_tab++;
                pos_1++;
                pos_2++;
                
                if(pos_2>lunghezza-1){
                    //out of index
                    if(y_tab==lunghezza-1){
                        y_tab=0;
                       
                        if(tabella[lunghezza-1][x_tab-1][0]=='O'){
                            tabella[lunghezza-1][x_tab-1][0]=' ';
                            
                        }
                        if (tabella[lunghezza-1][pos_1-1][0]=='O'){
                            tabella[lunghezza-1][pos_1-1][0]=' ';
                        }
                        if(tabella[lunghezza-1][pos_2-1][0]=='O'){
                            tabella[lunghezza-1][pos_2-1][0]=' ';
                        }
                        
                        
                    }else{
                        y_tab++;
                        
                        if(tabella[y_tab-1][x_tab-1][0]=='O'){
                            tabella[y_tab-1][x_tab-1][0]=' ';
                            
                        }
                        if (tabella[y_tab-1][pos_1-1][0]=='O'){
                            tabella[y_tab-1][pos_1-1][0]=' ';
                        }
                        if(tabella[y_tab-1][pos_2-1][0]=='O'){
                           tabella[y_tab-1][pos_2-1][0]=' ';
                        }
                        
                        

                    }
                    
                        
                    x_tab=0;
                    pos_1=1;
                    pos_2=2;
                    

                }
                
                if(tabella[y_tab][x_tab-1][0]!='X'){
                    tabella[y_tab][x_tab-1][0]=' ';
                }
                if (tabella[y_tab][pos_1-1][0]=='O'){
                    tabella[y_tab][pos_1-1][0]=' ';
                 }
                if(tabella[y_tab][pos_2-1][0]=='O'){
                     tabella[y_tab][pos_2-1][0]=' ';
                }
            
                
                
                if(tabella[y_tab][x_tab][0]!='X'&&tabella[y_tab][pos_1][0]!='X'&&tabella[y_tab][pos_2][0]!='X'){
                    tabella[y_tab][x_tab][0]='O';
                    tabella[y_tab][pos_1][0]='O';
                    tabella[y_tab][pos_2][0]='O';
                    
                }
                
                display(0);
            }else if(G_nave_att==2){
                //devo posizionare nave lunga 2
                
                x_tab++;
                pos_1++;
                
                
                if(pos_1>lunghezza-1){
                    //out of index
                    if(y_tab==lunghezza-1){
                        y_tab=0;
                       
                        if(tabella[lunghezza-1][x_tab-1][0]=='O'){
                            tabella[lunghezza-1][x_tab-1][0]=' ';
                            
                        }
                        if (tabella[lunghezza-1][pos_1-1][0]=='O'){
                            tabella[lunghezza-1][pos_1-1][0]=' ';
                        }
                        
                        
                        
                    }else{
                        y_tab++;
                        
                        if(tabella[y_tab-1][x_tab-1][0]=='O'){
                            tabella[y_tab-1][x_tab-1][0]=' ';
                            
                        }
                        if (tabella[y_tab-1][pos_1-1][0]=='O'){
                            tabella[y_tab-1][pos_1-1][0]=' ';
                        }
                        
                        
                        

                    }
                    
                        
                    x_tab=0;
                    pos_1=1;
                    
                    

                }
                if(tabella[y_tab][x_tab-1][0]!='X'){
                    tabella[y_tab][x_tab-1][0]=' ';
                }
                if (tabella[y_tab][pos_1-1][0]=='O'){
                    tabella[y_tab][pos_1-1][0]=' ';
                 }
                
                
                
                if(tabella[y_tab][x_tab][0]!='X'&&tabella[y_tab][pos_1][0]!='X'){
                    tabella[y_tab][x_tab][0]='O';
                    tabella[y_tab][pos_1][0]='O';
                
                    
                }
                
                display(0);
            }
            else{

            x_tab+=1;
            if(x_tab>lunghezza-1){
                if(tabella[y_tab][x_tab-1][0]=='O'){
                    tabella[y_tab][x_tab-1][0]= ' ';
                }
                
                x_tab=0;
                if(y_tab==lunghezza-1){
                    y_tab=0;
                }else
                {
                    y_tab+=1;
                }
                
                
                if(tabella[y_tab][x_tab][0]!='X'){
                    tabella[y_tab][x_tab][0]= 'O';
                }
                
            }else if(x_tab==0){
                
                if(tabella[y_tab][x_tab][0]!='X'){
                    tabella[y_tab][x_tab][0]= 'O';
                }
            }else{
                if(tabella[y_tab][x_tab-1][0]=='O'){
                    tabella[y_tab][x_tab-1][0]= ' ';
                }
                
                if(tabella[y_tab][x_tab][0]!='X'){
                    tabella[y_tab][x_tab][0]= 'O';
                }
            }
            
            display(0);
            }
        }
        if(ch == KEY_LEFT){
            
                x_tab-=1;
                if(x_tab<0){
                    if(tabella[y_tab][x_tab+1][0]=='O'){
                        tabella[y_tab][x_tab+1][0]= ' ';
                    }
                    
                    x_tab=lunghezza-1;
                    if(y_tab==0){
                        y_tab=lunghezza-1;
                    }else{
                        y_tab-=1;
                    }
                    
                    
                    if(tabella[y_tab][x_tab][0]!='X'){
                        tabella[y_tab][x_tab][0]= 'O';
                    }
                }else{
                    if(tabella[y_tab][x_tab+1][0]=='O'){
                        tabella[y_tab][x_tab+1][0]= ' ';
                    }
                    if(tabella[y_tab][x_tab][0]!='X'){
                        tabella[y_tab][x_tab][0]= 'O';
                    }
                }
                
                display(0);
            
            
        }
        if(ch == KEY_DOWN){
            
            y_tab+=1;
            if(y_tab>lunghezza-1){
                if(x_tab==lunghezza-1){
                    if(tabella[y_tab-1][x_tab][0]=='O'){
                    tabella[y_tab-1][x_tab][0]= ' ';
                }
                    y_tab=0;
                    x_tab=0;
                    
                    if(tabella[y_tab][x_tab][0]!='X'){
                    tabella[y_tab][x_tab][0]= 'O';
                }
                }else
                {
                    if(tabella[y_tab-1][x_tab][0]=='O'){
                    tabella[y_tab-1][x_tab][0]= ' ';
                }
                    x_tab+=1;
                    y_tab=0;
                    if(tabella[y_tab][x_tab][0]!='X'){
                    tabella[y_tab][x_tab][0]= 'O';
                }
                }
                
            }else{
                if(tabella[y_tab-1][x_tab][0]=='O'){
                    tabella[y_tab-1][x_tab][0]= ' ';
                }
                if(tabella[y_tab][x_tab][0]!='X'){
                    tabella[y_tab][x_tab][0]= 'O';
                }
            }
            
            display(0);
        }
        if(ch == KEY_UP){
            
            y_tab-=1;
            if(y_tab<0){
                if(x_tab==0){
                    if(tabella[y_tab+1][x_tab][0]=='O'){
                    tabella[y_tab+1][x_tab][0]= ' ';
                }
                    y_tab=lunghezza-1;
                    x_tab=lunghezza-1;
                    if(tabella[y_tab][x_tab][0]!='X'){
                    tabella[y_tab][x_tab][0]= 'O';
                }
                }else{
                    if(tabella[y_tab+1][x_tab][0]=='O'){
                    tabella[y_tab+1][x_tab][0]= ' ';
                }
                    y_tab=lunghezza-1;
                    x_tab-=1;
                    
                    if(tabella[y_tab][x_tab][0]!='X'){
                    tabella[y_tab][x_tab][0]= 'O';
                }
                }
                
            }else{
                if(tabella[y_tab+1][x_tab][0]=='O'){
                    tabella[y_tab+1][x_tab][0]= ' ';
                }
                if(tabella[y_tab][x_tab][0]!='X'){
                    tabella[y_tab][x_tab][0]= 'O';
                }
            }
            
            display(0);
        }
        if(ch == 10){
           if(G_nave_att==4){

               if(tabella[y_tab][x_tab][0]!='X'&&tabella[y_tab][pos_1][0]!='X'&&tabella[y_tab][pos_2][0]!='X'&&tabella[y_tab][pos_3][0]!='X'){
                   tabella[y_tab][x_tab][0]='X';
                   tabella[y_tab][pos_1][0]='X';
                   tabella[y_tab][pos_2][0]='X';
                   tabella[y_tab][pos_3][0]='X';
                    
                    lett=alf[lett_us];
                    lett_us++;

                    tabella[y_tab][x_tab][3]=lett;
                    tabella[y_tab][pos_1][3]=lett;
                    tabella[y_tab][pos_2][3]=lett;
                    tabella[y_tab][pos_3][3]=lett;

                    tabella[y_tab][x_tab][2]='A';
                    tabella[y_tab][pos_1][2]='A';
                    tabella[y_tab][pos_2][2]='A';
                    tabella[y_tab][pos_3][2]='A';

                    tabella[y_tab][x_tab][1]='D';
                    tabella[y_tab][pos_1][1]='D';
                    tabella[y_tab][pos_2][1]='D';
                    tabella[y_tab][pos_3][1]='D';
                        

               }else{
                   G_conta_tasti--;
               }

           }else if(G_nave_att==1){
               
               if(tabella[y_tab][x_tab][0]=='X'){
                G_conta_tasti--;
                }   
                tabella[y_tab][x_tab][0]='X';
                lett=alf[lett_us];
                    lett_us++;

                    tabella[y_tab][x_tab][3]=lett;
                   

                    tabella[y_tab][x_tab][2]='A';
                    

                    tabella[y_tab][x_tab][1]='A';
                    
           }else if(G_nave_att==3){

               if(tabella[y_tab][x_tab][0]!='X'&&tabella[y_tab][pos_1][0]!='X'&&tabella[y_tab][pos_2][0]!='X'){
                   tabella[y_tab][x_tab][0]='X';
                   tabella[y_tab][pos_1][0]='X';
                   tabella[y_tab][pos_2][0]='X';

                   lett=alf[lett_us];
                    lett_us++;

                    tabella[y_tab][x_tab][3]=lett;
                    tabella[y_tab][pos_1][3]=lett;
                    tabella[y_tab][pos_2][3]=lett;
                  

                    tabella[y_tab][x_tab][2]='A';
                    tabella[y_tab][pos_1][2]='A';
                    tabella[y_tab][pos_2][2]='A';
                    

                    tabella[y_tab][x_tab][1]='C';
                    tabella[y_tab][pos_1][1]='C';
                    tabella[y_tab][pos_2][1]='C';
           
                        
                   
               }else{
                   G_conta_tasti--;
               }

           }else if(G_nave_att==2){

               if(tabella[y_tab][x_tab][0]!='X'&&tabella[y_tab][pos_1][0]!='X'){
                   tabella[y_tab][x_tab][0]='X';
                   tabella[y_tab][pos_1][0]='X';

                   lett=alf[lett_us];
                    lett_us++;

                    tabella[y_tab][x_tab][3]=lett;
                    tabella[y_tab][pos_1][3]=lett;
            

                    tabella[y_tab][x_tab][2]='A';
                    tabella[y_tab][pos_1][2]='A';


                    tabella[y_tab][x_tab][1]='B';
                    tabella[y_tab][pos_1][1]='B';

                   
               }else{
                   G_conta_tasti--;
               }

           }
           
            
            
            
            display(0);
            break;
            
        }
        }else if(scelta==2){          //se la scelta è 2
            if(ch == KEY_RIGHT){
            
            x_tab+=1;
            if(x_tab>lunghezza-1){
                if(tabella_pl2[y_tab][x_tab-1]=='O'){
                    tabella_pl2[y_tab][x_tab-1]= ' ';
                }
                
                x_tab=0;
                if(y_tab==lunghezza-1){
                    y_tab=0;
                }else
                {
                    y_tab+=1;
                }
                
                
                if(tabella_pl2[y_tab][x_tab]!='X'){
                    tabella_pl2[y_tab][x_tab]= 'O';
                }
                
            }else if(x_tab==0){
                
                if(tabella_pl2[y_tab][x_tab]!='X'){
                    tabella_pl2[y_tab][x_tab]= 'O';
                }
            }else{
                if(tabella_pl2[y_tab][x_tab-1]=='O'){
                    tabella_pl2[y_tab][x_tab-1]= ' ';
                }
                
                if(tabella_pl2[y_tab][x_tab]!='X'){
                    tabella_pl2[y_tab][x_tab]= 'O';
                }
            }
            
            display(3);
        }
        if(ch == KEY_LEFT){
            
            x_tab-=1;
            if(x_tab<0){
                if(tabella_pl2[y_tab][x_tab+1]=='O'){
                    tabella_pl2[y_tab][x_tab+1]= ' ';
                }
                
                x_tab=lunghezza-1;
                if(y_tab==0){
                    y_tab=lunghezza-1;
                }else{
                    y_tab-=1;
                }
                
                
                if(tabella_pl2[y_tab][x_tab]!='X'){
                    tabella_pl2[y_tab][x_tab]= 'O';
                }
            }else{
                if(tabella_pl2[y_tab][x_tab+1]=='O'){
                    tabella_pl2[y_tab][x_tab+1]= ' ';
                }
                if(tabella_pl2[y_tab][x_tab]!='X'){
                    tabella_pl2[y_tab][x_tab]= 'O';
                }
            }
            
            display(3);
        }
        if(ch == KEY_DOWN){
            
            y_tab+=1;
            if(y_tab>lunghezza-1){
                if(x_tab==lunghezza-1){
                    if(tabella_pl2[y_tab-1][x_tab]=='O'){
                    tabella_pl2[y_tab-1][x_tab]= ' ';
                }
                    y_tab=0;
                    x_tab=0;
                    
                    if(tabella_pl2[y_tab][x_tab]!='X'){
                    tabella_pl2[y_tab][x_tab]= 'O';
                }
                }else
                {
                    if(tabella_pl2[y_tab-1][x_tab]=='O'){
                    tabella_pl2[y_tab-1][x_tab]= ' ';
                }
                    x_tab+=1;
                    y_tab=0;
                    if(tabella_pl2[y_tab][x_tab]!='X'){
                    tabella_pl2[y_tab][x_tab]= 'O';
                }
                }
                
            }else{
                if(tabella_pl2[y_tab-1][x_tab]=='O'){
                    tabella_pl2[y_tab-1][x_tab]= ' ';
                }
                if(tabella_pl2[y_tab][x_tab]!='X'){
                    tabella_pl2[y_tab][x_tab]= 'O';
                }
            }
            
            display(3);
        }
        if(ch == KEY_UP){
            
            y_tab-=1;
            if(y_tab<0){
                if(x_tab==0){
                    if(tabella_pl2[y_tab+1][x_tab]=='O'){
                    tabella_pl2[y_tab+1][x_tab]= ' ';
                }
                    y_tab=lunghezza-1;
                    x_tab=lunghezza-1;
                    if(tabella_pl2[y_tab][x_tab]!='X'){
                    tabella_pl2[y_tab][x_tab]= 'O';
                }
                }else{
                    if(tabella_pl2[y_tab+1][x_tab]=='O'){
                    tabella_pl2[y_tab+1][x_tab]= ' ';
                }
                    y_tab=lunghezza-1;
                    x_tab-=1;
                    
                    if(tabella_pl2[y_tab][x_tab]!='X'){
                    tabella_pl2[y_tab][x_tab]= 'O';
                }
                }
                
            }else{
                if(tabella_pl2[y_tab+1][x_tab]=='O'){
                    tabella_pl2[y_tab+1][x_tab]= ' ';
                }
                if(tabella_pl2[y_tab][x_tab]!='X'){
                    tabella_pl2[y_tab][x_tab]= 'O';
                }
            }
            
            display(3);
        }
        if(ch == 10){
           
            if(tabella_pl2[y_tab][x_tab]=='X'){
                G_conta_tasti--;
            }
            
            
            tabella_pl2[y_tab][x_tab]='X';
            display(3);
            break;
            
        }
        }
        
        
    }
}
void a_navi(int scelta){
    /*
         _______________________________________________________________________
        |                                                                       |
        |   LEGENDA FUNZIONE                                                    |
        |        - NESSUN PARAMETRO IN INPUT                                    |
        |                                                                       |
        |        - RICHIEDI IL POSIZIONAMENTO DELLE NAVI DA PARTE DEL PLAYER    |
        |                                                                       |
        |                                                                       |
        |_______________________________________________________________________|

    */ 
    
   
    int navi_4=1, navi_3=3, navi_2=6, navi_1=10, i=0;

    if(scelta==1){
        G_conta_tasti=0;
        x_tab=0;
        y_tab=0;
        tabella_pl2[0][0]= 'O';     //imposto il cursore a 0 0
        display(3);     //mostro la tabella con parametro 0 per avere solo la tabella con le navi del player
        while(G_conta_tasti < n_navi)    //finchè la variabile 'globale' G_conta_tasti è minore del numero delle navi    
        {
            
            tasti(2);       //richiamo la funzione tasti con parametro 0 per il posizionamento navi
            
        G_conta_tasti+=1;
        }
    }else{
        G_conta_tasti=0;
        x_tab=0;
        y_tab=0;
        tabella[0][0][0]= 'O';     //imposto il cursore a 0 0
        display(0);     //mostro la tabella con parametro 0 per avere solo la tabella con le navi del player
        while(G_conta_tasti < n_navi)    //finchè la variabile 'globale' G_conta_tasti è minore del numero delle navi    
        {
            if (single==true){
                if(i<navi_4){
                    G_nave_att=4;
                }else if(i<navi_3){
                    G_nave_att=3;
                }else if (i<navi_2){
                    G_nave_att=2;
                }else{
                    G_nave_att=1;
                }
            }else
            {
                G_nave_att=1;
            }
            

            
            tasti(0);       //richiamo la funzione tasti con parametro 0 per il posizionamento navi
            ++i;
        G_conta_tasti+=1;
        }
    }
    
    
    
    
}
void display(int scelta){ //scelta può essere 0 , 1 , 2, 3, 4,  5
/*
         _______________________________________________________________________
        |                                                                       |
        |   LEGENDA FUNZIONE                                                    |
        |        - PARAMETRO SCELTA IN INPUT                                    |
        |                                                                       |
        |        - MOSTRA A VIDEO LA TABELLA DI GIOCO                           |
        |               - IN BASE AL VALORE DI SCELTA SI MOSTRANO DELLE FRASI   |
        |                                                                       |
        |_______________________________________________________________________|

    */ 
    int i=0, j=0, k=0;      //dichiaro ed inizializzo variabili necessarie
    erase();        //pulisco lo schermo con una funzione di 
    if(scelta==2||scelta==4){      // se il valore di scelta è 2 
        frase("Scegli dove attaccare il nemico...");        //richiamo la funzione frase che scrive a video la stringa in input con animazione
    }else if(scelta==1||scelta==5){                                //se la scelta è 1
        printw("Scegli dove attaccare il nemico...\n");       //viene riscritto ma senza animazione altrimenti ogni volta che clicco una freccia mi farebbe l'animazione
    }
    
    if(scelta==0||scelta==1||scelta==2){

    printw( "  ");
    for(k=0;k<lunghezza;k++)    //for che cicla per la lunghezza     
        {
            
            printw("   %d  ",k);
        }
        printw("\n");
        printw( "  ");
    for(k=0;k<lunghezza;k++)    //for che cicla per la lunghezza     
        {
            
            printw("______");
        }
        printw("\n");
    for(i=0;i<lunghezza;i++){
        
        printw("%c ",alf[i]);
        for(j=0;j<lunghezza;j++){
            if(scelta==1||scelta==2){
                printw("|  %c  ", tabella_spari[i][j]);
            }else{
                printw("|  %c  ", tabella[i][j][0]);
            }
                
        }
        printw("|");
        printw("\n");
        printw("  ");
        for(k=0;k<lunghezza;k++)
        {
            printw("______");
        }
        
        
        printw("\n");
    }
    printw("\n");

    }else{
        printw( "  ");
        for(k=0;k<lunghezza;k++)    //for che cicla per la lunghezza     
        {
            
            printw("______");
        }
        printw("\n");
    for(i=0;i<lunghezza;i++){
        
        printw("%c ",alf[i]);
        for(j=0;j<lunghezza;j++){
            if(scelta==4||scelta==5){
                printw("|  %c  ", tabella_spari_pl2[i][j]);
            }else{
                printw("|  %c  ", tabella_pl2[i][j]);
            }
                
        }
        printw("|");
        printw("\n");
        printw("  ");
        for(k=0;k<lunghezza;k++)
        {
            printw("______");
        }
        
        
        printw("\n");
    }
    printw("\n");
    }

    // stampo la tabella in base alla sua dimensione
    //-------------------------------------------------------------------------------
    
    //-------------------------------------------------------
    refresh();      //richiamo la funzione refresh della libreria ncurses
}

void scelta_cas(void){
    /*
         _______________________________________________________________________
        |                                                                       |
        |   LEGENDA FUNZIONE                                                    |
        |        - NESSUN PARAMETRO IN INPUT                                    |
        |                                                                       |
        |        - RICHIEDI IL POSIZIONAMENTO AL PLAYER DOVE TENTARE DI COLPIRE |
        |             LA NAVE NEMICA                                            |
        |                                                                       |
        |_______________________________________________________________________|

    */ 
    display(2);
    x_tab=0;
    y_tab=0;
    tabella_spari[0][0]= 'O';
    tasti(1);    
    frase("Lancio missile in corso ...");
    sleep_ms(2000);  
    verifica(y_tab, x_tab, 1);

}

void verifica(int riga, int colonna, int valore){
    /*
         _______________________________________________________________________
        |                                                                       |
        |   LEGENDA FUNZIONE                                                    |
        |        - INPUT: RIGA, COLONNA, VALORE                                 |
        |                                                                       |
        |        - VERIFICA SE IL COLPO SPARATO HA COLPITO UNA NAVE             |
        |                                                                       |
        |                                                                       |
        |_______________________________________________________________________|

    */ 
    if(valore==1){     //se il valore è 1
        if (tabella_bot[riga][colonna]=='X') {
            tabella_bot[riga][colonna]='F';
            
            display(0);
            frase("Nave affondata!");
            refresh();
            sleep_ms(1000);
            navi_aff_2+=1;
            
        }else{
            display(0);
            frase("Non hai colpito la nave!");
            refresh();
            sleep_ms(1000);
        }
    }else if (valore==0){      //altrimenti il valore 0
        if (tabella[riga][colonna][0]=='X') {
            tabella[riga][colonna][0]='F';
            display(0);
            sleep_ms(500);
            erase();
            printw("Attenzione: La tua nave è stata affondata \n");
            refresh();
            sleep_ms(1500);
            navi_aff_1+=1;
            
            
        }else{
            sleep_ms(500);
            erase();
            printw("Il nemico ha fatto cilecca \n");
            refresh();
            sleep_ms(1500);
        }
    }else{
        if(tabella[riga][colonna][0]=='X'){
            switch(tabella[riga][colonna][1]){
                case 'A':
                    //lunga 1
                    tabella[riga][colonna][0]='F';
                    display(0);
                    sleep_ms(500);
                    erase();
                    printw("Attenzione: La tua nave è stata affondata \n");
                    refresh();
                    sleep_ms(1500);
                    navi_aff_1+=1;
                    break;
                case 'B':
                    if(tabella[riga][colonna][2]=='D'){
                        //danno al 50%
                        for(int i =0; i<lunghezza; i++){
                            for(int j =0; j<lunghezza; j++){
                                if(tabella[i][j][3]==tabella[riga][colonna][3]){
                                    tabella[i][j][2]='E';
                                    tabella[i][j][0]='F';
                                }
                            }
                        }
                        display(0);
                        sleep_ms(500);
                        erase();
                        printw("Attenzione: La tua nave è stata affondata \n");
                        refresh();
                        sleep_ms(1500);
                        navi_aff_1+=1;
                    }else{
                        //danno 100%
                        for(int i =0; i<lunghezza; i++){
                            for(int j =0; j<lunghezza; j++){
                                if(tabella[i][j][3]==tabella[riga][colonna][3]){
                                    tabella[i][j][2]='C';
                                    tabella[i][j][0]='C';
                                }
                            }
                        }
                        display(0);
                        sleep_ms(500);
                        erase();
                        printw("Attenzione: La tua nave è stata colpita \n");
                        refresh();
                        sleep_ms(1500);
                        
                    }
                    break;
                case 'C':
                    if(tabella[riga][colonna][2]=='B'){
                        //danno al 66,6%
                        for(int i =0; i<lunghezza; i++){
                            for(int j =0; j<lunghezza; j++){
                                if(tabella[i][j][3]==tabella[riga][colonna][3]){
                                    tabella[i][j][2]='D';
                                    tabella[i][j][0]='C';
                                }
                            }
                        }
                        display(0);
                        sleep_ms(500);
                        erase();
                        printw("Attenzione: La tua nave è stata colpita \n");
                        refresh();
                        sleep_ms(1500);
                        
                    }else if(tabella[riga][colonna][2]=='D'){
                        for(int i =0; i<lunghezza; i++){
                            for(int j =0; j<lunghezza; j++){
                                if(tabella[i][j][3]==tabella[riga][colonna][3]){
                                    tabella[i][j][2]='E';
                                    tabella[i][j][0]='F';
                                }
                            }
                        }
                        display(0);
                        sleep_ms(500);
                        erase();
                        printw("Attenzione: La tua nave è stata affondata \n");
                        refresh();
                        sleep_ms(1500);
                        navi_aff_1+=1;
                    }else{
                        for(int i =0; i<lunghezza; i++){
                            for(int j =0; j<lunghezza; j++){
                                if(tabella[i][j][3]==tabella[riga][colonna][3]){
                                    tabella[i][j][2]='B';
                                    tabella[i][j][0]='C';
                                }
                            }
                        }
                        display(0);
                        sleep_ms(500);
                        erase();
                        printw("Attenzione: La tua nave è stata colpita \n");
                        refresh();
                        sleep_ms(1500);
                    }
                    break;

                case 'D':
                    if(tabella[riga][colonna][2]=='A'){
                        for(int i =0; i<lunghezza; i++){
                            for(int j =0; j<lunghezza; j++){
                                if(tabella[i][j][3]==tabella[riga][colonna][3]){
                                    tabella[i][j][2]='F';
                                    tabella[i][j][0]='C';
                                }
                            }
                        }
                        display(0);
                        sleep_ms(500);
                        erase();
                        printw("Attenzione: La tua nave è stata colpita \n");
                        refresh();
                        sleep_ms(1500);
                    }else if(tabella[riga][colonna][2]=='F'){
                        //danno 50%
                        for(int i =0; i<lunghezza; i++){
                            for(int j =0; j<lunghezza; j++){
                                if(tabella[i][j][3]==tabella[riga][colonna][3]){
                                    tabella[i][j][2]='C';
                                    tabella[i][j][0]='C';
                                }
                            }
                        }
                        display(0);
                        sleep_ms(500);
                        erase();
                        printw("Attenzione: La tua nave è stata colpita \n");
                        refresh();
                        sleep_ms(1500);
                    }else if(tabella[riga][colonna][2]=='C'){
                        for(int i =0; i<lunghezza; i++){
                            for(int j =0; j<lunghezza; j++){
                                if(tabella[i][j][3]==tabella[riga][colonna][3]){
                                    tabella[i][j][2]='G';
                                    tabella[i][j][0]='C';
                                }
                            }
                        }
                        display(0);
                        sleep_ms(500);
                        erase();
                        printw("Attenzione: La tua nave è stata colpita \n");
                        refresh();
                        sleep_ms(1500);
                    }else{
                        for(int i =0; i<lunghezza; i++){
                            for(int j =0; j<lunghezza; j++){
                                if(tabella[i][j][3]==tabella[riga][colonna][3]){
                                    tabella[i][j][2]='E';
                                    tabella[i][j][0]='F';
                                }
                            }
                        }
                        display(0);
                        sleep_ms(500);
                        erase();
                        printw("Attenzione: La tua nave è stata affondata \n");
                        refresh();
                        sleep_ms(1500);
                        navi_aff_1+=1;
                    }
                    break;        
            }
        }else{
            sleep_ms(500);
            erase();
            printw("Il nemico ha fatto cilecca \n");
            refresh();
            sleep_ms(1500);
        }
    }
    
    
    
}
void emit_beep(void){
    /*
         _______________________________________________________________________
        |                                                                       |
        |   LEGENDA FUNZIONE                                                    |
        |        - NESSUN PARAMETRO IN INPUT                                    |
        |                                                                       |
        |        - EMETTE UN BEEP SONORO O UN LAPEGGIO DEL TERMINALE            |
        |                                                                       |
        |                                                                       |
        |_______________________________________________________________________|

    */ 
    printf("\a");
}


void frase(char frase[50]){
    /*
         _______________________________________________________________________
        |                                                                       |
        |   LEGENDA FUNZIONE                                                    |
        |        - IN INPUT LA FRASE                                            |
        |                                                                       |
        |        - SCRIVE A VIDEO LA FRASE IN INPUT CON ANIMAZIONE              |
        |                                                                       |
        |                                                                       |
        |_______________________________________________________________________|

    */ 
    int lung= strlen(frase), i=0;
    for (i = 0; i < lung; i++)
    {
        printw("%c", frase[i]);
        refresh();
        sleep_ms(50);
    }
    printw("\n");
    refresh();
    
    
    
}
void sleep_ms(int millisecondi) //funzione multipiattaforma per lo sleep del thread prendendo in ingresso i millisecondi
{
    /*
         _______________________________________________________________________
        |                                                                       |
        |   LEGENDA FUNZIONE                                                    |
        |        - MILLISECONDI IN INPUT                                        |
        |                                                                       |
        |        - SLEEP                                                        |
        |                                                                       |
        |                                                                       |
        |_______________________________________________________________________|

    */ 
#ifdef WIN32
    Sleep(millisecondi);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = millisecondi / 1000;
    ts.tv_nsec = (millisecondi % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(millisecondi * 1000);
#endif
}


int menu(void) {
    char cursore='O',cursore1=' ';
    int scelta=0, ch=0;
     int a=0;
    
   frase ("Vuoi giocare singleplayer o multiplayer?");
    while (1){
        if(scelta==0){
            erase();
            printw("Vuoi giocare singleplayer o multiplayer?\n ");
            printw("___\n");
            printw("|%c| SinglePlayer\n",cursore);
            printw("---\n");

            printw("___\n");
            printw("|%c| MultiPlayer\n",cursore1);
            printw("---\n");
            refresh();
            scelta=1;
        }
        ch=getch();
        if (ch==KEY_UP||ch==KEY_DOWN){
            if (cursore=='O'){
                cursore=' ';
                cursore1='O';
            }else {
                cursore='O';
                cursore1=' ';
            }
            scelta=0;
        }

         if (ch==10){
           
            if (cursore=='O') {
                a=0;
            }else {
                a=1;
            }
            break;
        }
    }
    

    return a;
}

void scelta_bot(void){
    int x = (rand() % (lunghezza-1  + 1));
    int y = (rand() % (lunghezza-1  + 1));
    sleep_ms(500);
    printw("Il computer ti sta attaccando...\n ");
    refresh();
    sleep_ms(1000);
    verifica(x, y, 2);

    

}



int finita(int val){
    
    int scelta=0, a=0, ch=0;
    char cursore='O', cursore1=' ';

    if(val==1){
        erase();
        frase("PLAYER 1:");
        frase("COMPLIMENTI HAI VINTO LA PARTITA");
     
    }else{
        erase();
        frase("PLAYER 2:");
        frase("COMPLIMENTI HAI VINTO LA PARTITA");
    }
    while (1){
        if(scelta==0){
           
            printw("Vuoi ripetere?\n");
            printw("___\n");
            printw("|%c| Sì\n",cursore);
            printw("---\n");

            printw("___\n");
            printw("|%c| No\n",cursore1);
            printw("---\n");
            refresh();
            scelta=1;
        }
        ch=getch();
        if (ch==KEY_UP||ch==KEY_DOWN){
            if (cursore=='O'){
                cursore=' ';
                cursore1='O';
            }else {
                cursore='O';
                cursore1=' ';
            }
            scelta=0;
        }

         if (ch==10){
           
            if (cursore=='O') {
                a=0;
            }else {
                a=1;
            }
            break;
        }
    }
    
    return a;
}