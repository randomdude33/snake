#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <time.h>
#include <cstdlib> //generazione di numeri casuali
#include <SDL_ttf.h> //per scrivere nella finestra 
#include <windows.h> //usato per far aspettare il programma


#define LUNG_FINESTRA 660
#define ALT_FINESTRA 660
#define MIN_TIME 4 //tempo minimo trascorso prima di svolgere un nuovo movimento
#define RIGHE_MAPPA 15 //caselle del quadrato
#define ALT_SNAKE 44
#define MAX_CORPO 125


//inizializzazione della finestra, renderer e texture utilizzati
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;


bool INIT();
void titlescreen();
void game();
void mela(int &x_mela, int &y_mela, bool& stato_mela, int &mele_mangiate, int &x_testa, int &y_testa, int &ritardo, int(&x_corpo)[MAX_CORPO], int(&y_corpo)[MAX_CORPO]);
void testa(int direzione_testa, int &direzione_attuale, int &x_testa, int &y_testa, int(&ordine_coda)[MAX_CORPO], bool &fine_gioco);
void coda(int &x_coda, int &y_coda, int& direzione_coda, int &ritardo, int (& ordine_coda)[MAX_CORPO], int &spazio_mancante);
void corpo(int& x_testa, int& y_testa, int (& x_corpo)[MAX_CORPO], int(&y_corpo)[MAX_CORPO], int &mele_mangiate, bool &fine_gioco);
void punteggio(int& mele_mangiate, int &flag_punteggio);
bool time_difference(int last_time);
void update_testa_coda_mela(SDL_Texture* texture, int& x_texture, int& y_texture, SDL_Rect& rect_texture);
void elimina_texture(SDL_Texture* texture_testa, SDL_Texture* texture_corpo, SDL_Texture* texture_coda, SDL_Texture* texture_mela);
void end();


int main(int argc, char* args[]) 
{
    //imposto il seed per la generazione dei numeri casuali
    srand((unsigned)time(NULL));

    if (!INIT())
    {

        printf("Could not initialize.\n");
        return -1;
    }
    else
    {
        game();
    }

    //chiudo "funzioni" aperte e libero la memoria utilizzata
    end();

	return 0;
}


bool INIT()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) //inizializzazione del viseo se è minore di 0 qualcosa è andato storto
    {
        std::cout << "impossibile far andare il video." << SDL_GetError << std::endl; //qualche informazione in più
    }
    else
    {
        window = SDL_CreateWindow("SNAKE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LUNG_FINESTRA, ALT_FINESTRA, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            std::cout << "Errore creazione finestra" << SDL_GetError << std::endl; //controllo finestra
            SDL_DestroyWindow(window); //libero lo spazio della finestra
        }

    }


    //inizializzo la renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //controllo
    if (renderer == NULL)
    {
        std::cout << "Impossibile to create the renderer.\n" << SDL_GetError();
        return false;
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL image could not be initialized. SDL_image error: %s \n", IMG_GetError());
    }

    return true;
}

SDL_Texture* LoadTexture(std::string file)
{
    SDL_Texture* newTexture = NULL;
    SDL_Surface* loadSurface = IMG_Load(file.c_str());
    if (loadSurface == NULL)
    {
        printf("Unable to load image %s! SDL_image error: %s\n", file.c_str(), IMG_GetError());
    }
    else
    {
        if (renderer == NULL)
        {
            std::cout << "Sono nullo";
        }

        newTexture = SDL_CreateTextureFromSurface(renderer, loadSurface);

        if (newTexture == NULL)
        {
            printf("Unable to create the texture from %s! SDL_error: %s\n", file.c_str(), SDL_GetError());
        }
        SDL_FreeSurface(loadSurface);
    }

    return newTexture;
}


void titlescreen()
{
    //funzione non utilizzata
    SDL_Texture* texture2 = NULL;
    texture = LoadTexture("immagini/dice.png");
   

    SDL_Rect rect1{}; rect1.x = 300 / 2; rect1.y = 300 / 2; rect1.w = 100; rect1.h = 100;


    bool isrunning = true; //variabili per il loop
    SDL_Event event;
    //utilizzato per verificare se sia passato un tempo minimo -NON METTERLO UGUALE A 0-
    int last_time = 1;

    while (isrunning == true) //main loop
    {
            while (SDL_PollEvent(&event) != 0) //finchè la coda di eventi non è 0 fai 
            {
                if (event.type == SDL_QUIT)
                {
                    isrunning = false;
                }

                if (event.type == SDL_KEYDOWN)
                {
                    if (event.key.keysym.sym == SDLK_RETURN)
                    {
                        
                            //chiudo il ciclo pulisco la renderer e passo al gioco vero e proprio
                            isrunning = false;
                            SDL_DestroyTexture(texture);
                            SDL_DestroyTexture(texture2);
                            SDL_RenderClear(renderer);
                            game();
                        
                    }
                }

            }

            //Clear screen necessari per aggiornare la texture eccetera
            SDL_RenderClear(renderer);
            //Render texture to screen
            SDL_RenderCopy(renderer, texture, NULL, NULL);

            SDL_RenderCopy(renderer, texture2, NULL, &rect1);
            //Update screen
            SDL_RenderPresent(renderer);
        }
}


void game()
{
    Sleep(500);
    //dichiaro le texture utilizzate e gli assegno le rispettive immagini
    SDL_Texture* texture_testa = NULL;
    SDL_Texture* texture_mela = NULL;
    SDL_Texture* texture_coda = NULL;
    SDL_Texture* texture_corpo = NULL;

    texture = LoadTexture("immagini/background.png");
    texture_testa = LoadTexture("immagini/testa sopra.png");
    texture_mela = LoadTexture("immagini/mela.png");
    texture_coda = LoadTexture("immagini/coda.png");
    texture_corpo = LoadTexture("immagini/corpo.png");
    

    //texture dello snake
    SDL_Rect rect_testa{}; 
    rect_testa.x = (LUNG_FINESTRA-ALT_SNAKE)/2; 
    rect_testa.y = (ALT_FINESTRA - ALT_SNAKE) / 2;
    rect_testa.w = ALT_SNAKE; rect_testa.h = ALT_SNAKE;
    //texture della mela
    SDL_Rect rect_mela{};
    rect_mela.w = ALT_SNAKE; rect_mela.h = ALT_SNAKE;
    //texture della coda
    SDL_Rect rect_coda{};
    rect_coda.w = ALT_SNAKE; rect_coda.h = ALT_SNAKE;
    
    //variabili per l'avanzamento dopo un lasso di tempo determinato
    int last_time = 1;

    //varaiabili per il serpente
    int x_testa = (LUNG_FINESTRA-ALT_SNAKE)/2;
    int y_testa = (ALT_FINESTRA - ALT_SNAKE) / 2;
    int direzione_testa = 1;
    int direzione_attuale = 1;

    //variabili per il corpo
    int x_corpo[MAX_CORPO]{ (ALT_FINESTRA - ALT_SNAKE) / 2 };
    int y_corpo[MAX_CORPO]{ (ALT_FINESTRA - ALT_SNAKE) / 2 };
    SDL_Rect rect_corpo{};
    rect_corpo.w = ALT_SNAKE;
    rect_corpo.h = ALT_SNAKE;


    //variabili coda
    int direzione_coda = 1;
    int x_coda = (LUNG_FINESTRA - ALT_SNAKE) / 2;  
    int y_coda = (ALT_FINESTRA - ALT_SNAKE) / 2 + ALT_SNAKE;
    int ritardo = -ALT_SNAKE;
    int ordine_coda[MAX_CORPO]{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    int spazio_mancante = 0;
    //imposto la partenza della coda
    rect_coda.x = x_coda;
    rect_coda.y = y_coda;

   
    //variabili per la mela
    int x_mela = 0; int y_mela = 0;
    bool stato_mela = false;//stabilisce se la mela è presente
    int mele_mangiate = 0;

    bool fine_gioco = false;
    //necessario per ottenere lo stato dei tasti
    const Uint8* keystates = SDL_GetKeyboardState(NULL);
    SDL_Event event;

    bool isrunning = true; //variabili per il loop
    while (isrunning == true) //main loop
    {
        //genera le mele da mangiare
        mela(x_mela, y_mela, stato_mela, mele_mangiate, x_testa, y_testa, ritardo, x_corpo, y_corpo);

        enum direzione
        {
            fermo = 0, sopra = 1, sotto = 2, destra = 3, sinistra = 4
        };

        while (SDL_PollEvent(&event) != 0) //finchè la coda di eventi non è 0 fai 
        {
            if (event.type == SDL_QUIT)
            {
                 isrunning = false;
            }

            if ((keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_UP]))
            {
                direzione_testa = sopra;
            }
            else if (keystates[SDL_SCANCODE_S] || keystates[SDL_SCANCODE_DOWN])
            {
                direzione_testa = sotto;
            }
            else if (keystates[SDL_SCANCODE_D] || keystates[SDL_SCANCODE_RIGHT])
            {
                direzione_testa = destra;
            }
            else if (keystates[SDL_SCANCODE_A] || keystates[SDL_SCANCODE_LEFT])
            {
                direzione_testa = sinistra;
            }
        }
        
        //controlla se il gioco è terminato
        if (fine_gioco == true)
        {
            //fermo il programma per un attimo per far capire di aver perso
            Sleep(500);

            //pulisco la finestra e elimino le texture che non uso più
            SDL_RenderClear(renderer);
            elimina_texture(texture_testa, texture_corpo, texture_coda, texture_mela);
            int flag_punteggio = 0;
   
            //stampo il punteggio
            punteggio(mele_mangiate, flag_punteggio); 

            break;
        }
        else
        {

            if (time_difference(last_time) == true)
            {
                //se un lasso di tempo determinato è passato sposta la testa e la coda
                last_time = clock();
                testa(direzione_testa, direzione_attuale, x_testa, y_testa, ordine_coda, fine_gioco);
                corpo(x_testa, y_testa, x_corpo, y_corpo, mele_mangiate, fine_gioco);
                coda(x_coda, y_coda, direzione_coda, ritardo, ordine_coda, spazio_mancante);
            }

            //rimuove tutto dalla renderer
            SDL_RenderClear(renderer);

            //aggiorna lo sfondo
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            
            //aggiorno il corpo
            for (int i = 0; i < mele_mangiate + 1; i++)
            {
                rect_corpo.x = x_corpo[i];
                rect_corpo.y = y_corpo[i];
                //applica la texture allo scermo
                SDL_RenderCopy(renderer, texture_corpo, NULL, &rect_corpo);
            }
            //aggiorno la testa
            switch (direzione_testa)
            {
            case sopra:
                texture_testa = LoadTexture("immagini/testa sopra.png");
                break;
            case sotto:
                texture_testa = LoadTexture("immagini/testa sotto.png");
                break;
            case destra:
                texture_testa = LoadTexture("immagini/testa destra.png");
                break;
            case sinistra:
                texture_testa = LoadTexture("immagini/testa sinistra.png");
                break;
            }
            
            
            update_testa_coda_mela(texture_mela, x_mela, y_mela, rect_mela);
            update_testa_coda_mela(texture_testa, x_testa, y_testa, rect_testa);
            update_testa_coda_mela(texture_coda, x_coda, y_coda, rect_coda);
            
            //distruggo la texture della testa per evitare di avere una "memory leak"
            SDL_DestroyTexture(texture_testa);

            //Update screen
            SDL_RenderPresent(renderer);
        }    
    }
    //libera lo spazio assegnato alle texture
    elimina_texture(texture_testa, texture_corpo, texture_coda, texture_mela);   
}


void mela(int &x_mela, int &y_mela, bool &stato_mela, int& mele_mangiate, int& x_testa, int& y_testa, int& ritardo, int (&x_corpo)[MAX_CORPO], int(&y_corpo)[MAX_CORPO])
{
    //flag per controllare se la mela è stata posizionata sul serpente
    int flag_mela = 0;

    if (stato_mela == false && mele_mangiate==0)
    {
        stato_mela = true;
        
        do
        {
            x_mela = rand() % (RIGHE_MAPPA);//una casella x casuale viene scelta
            y_mela = rand() % (RIGHE_MAPPA);//una casella y casuale viene scelta
            
        } while (x_mela!=8 && y_mela!=8); //entrambe diverse da 8
        
        x_mela = x_mela * ALT_SNAKE; // rendo le coordinate utilizzabili
        y_mela = y_mela * ALT_SNAKE;  
    }
    else if(stato_mela==false && mele_mangiate!=0)
    {
        stato_mela = true;
        
        //controllo che la mela non venga posizionata sul serpente
        do
        {
            flag_mela = 0;

            x_mela = rand() % (RIGHE_MAPPA);//una casella x casuale viene scelta
            y_mela = rand() % (RIGHE_MAPPA);//una casella y casuale viene scelta
            for (int i = 0; i < mele_mangiate+1; i++)
            {
                if (x_mela==x_corpo[i])
                {
                    flag_mela = 1;
                }
            }

        } while (flag_mela==1);
        

        x_mela = x_mela * ALT_SNAKE; // rendo le coordinate utilizzabili
        y_mela = y_mela * ALT_SNAKE;
    }
    else
    {
        if (x_mela==x_testa && y_mela==y_testa)
        {
            stato_mela = false;
            //aggiorno il contatore delle mele mangiate e aumento il ritardo della coda
            mele_mangiate++;
            //aumento il ritardo della coda
            ritardo=ritardo+44;
        }       
    }
}


void testa(int direzione_testa, int &direzione_attuale, int &x_testa, int &y_testa, int(&ordine_coda)[MAX_CORPO], bool& fine_gioco)
{
    /*direzione_testa è la direzione verso cui sta andando il serpente
    mentre direzione attuale è quella in cui vuoi cambiare */
    enum direzione
    {
        fermo=0, sopra = 1, sotto = 2, destra = 3, sinistra = 4
    };

    //all'inizio di ogni casella
    if (x_testa%ALT_SNAKE==0 && y_testa%ALT_SNAKE==0)
    {
        //controlla che tu non voglia cambiare alla direzione opposta
        if ((direzione_attuale==sopra && direzione_testa==sotto) || (direzione_attuale==sotto && direzione_testa==sopra)
            || (direzione_attuale==destra && direzione_testa==sinistra)
            || (direzione_attuale==sinistra && direzione_testa==destra)){}
        else
        {
            //cambio la direzione della testa che viene applicata alla prossima occasione
            direzione_attuale = direzione_testa;

            //aggiorno le azioni della coda mettendole nella prima casella disponibile
            for (int i = 0; i < MAX_CORPO; i++)
            {
                if (ordine_coda[i]==0)
                {
                    ordine_coda[i] = direzione_attuale;
                    break;
                }
            }
            
        }    
    }

    if (direzione_attuale == sopra )
    {
        if (y_testa>0)
        {
            y_testa = y_testa--;
        }
        else
        {
            direzione_testa = fermo;
            fine_gioco = true;
        }
    }
    else if(direzione_attuale ==sotto)
    {
        if (y_testa<=ALT_FINESTRA-ALT_SNAKE)
        {
            y_testa = y_testa++;
        }
        else
        {
            direzione_testa = fermo;
            fine_gioco = true;
        }
    }
    else if (direzione_attuale == destra)
    {
        if (x_testa <= LUNG_FINESTRA-ALT_SNAKE)
        {
            x_testa = x_testa++;
        }
        else
        {
            direzione_testa = fermo;
            fine_gioco = true;
        }      
    }
    else if (direzione_attuale == sinistra)
    {
        if (x_testa!=0)
        {
            x_testa = x_testa--;
        }
        else
        {
            direzione_testa = fermo;
            fine_gioco = true;
        }      
    }
    else if(direzione_attuale == fermo)
    {
        fine_gioco = true;
    }
}


void coda(int &x_coda, int &y_coda,int &direzione_coda, int &ritardo, int (&ordine_coda)[MAX_CORPO], int &spazio_mancante)
{
    enum direzione //elenco direzioni
    {
        fermo = 0, sopra = 1, sotto = 2, destra = 3, sinistra = 4
    };

    if(spazio_mancante == 0)
    {
        spazio_mancante = spazio_mancante + ALT_SNAKE;
        direzione_coda = ordine_coda[0];
        for (int i = 0; i < 24; i++)
        {
            ordine_coda[i] = ordine_coda[i + 1];

        }
     }


    if (ritardo<0)
    {
        y_coda--;
        ritardo++;
    }
    else if (ritardo>0)
    {
        ritardo--;
    }
    else
    {
        if (spazio_mancante>0)
        {
            spazio_mancante--;
            if (direzione_coda==sopra)
            {
                y_coda--;
            }
            else if(direzione_coda==sotto)
            {
                y_coda++;
            }
            else if(direzione_coda==destra)
            {
                x_coda++;
            }
            else if (direzione_coda==sinistra)
            {
                x_coda--;
            }
            else
            {

            }
        }
    }
}


void corpo(int& x_testa, int& y_testa, int(&x_corpo)[MAX_CORPO], int(&y_corpo)[MAX_CORPO], int &mele_mangiate, bool &fine_gioco)
{
    //se si trova in una nuova casella aggiorno l'elenco di caselle occupate
    if (x_testa%ALT_SNAKE==0 && y_testa%ALT_SNAKE==0)
    {
        //std::cout << " coord testa " << std::endl;
        if (x_corpo[0] != x_testa || y_corpo[0] != y_testa) {
            for (int i = MAX_CORPO-1; i > 0; i--)
            {
                x_corpo[i] = x_corpo[i-1];
                y_corpo[i] = y_corpo[i-1];
            }

            x_corpo[0] = x_testa;
            y_corpo[0] = y_testa;
        }          
    }

    //controllo collisione
    if (x_testa % ALT_SNAKE == 0 && y_testa % ALT_SNAKE == 0)
    {
        for (int i = 1; i < mele_mangiate + 2; i++)
        {
            if (x_testa == x_corpo[i] && y_testa == y_corpo[i])
            {
                fine_gioco = true;
                break;
            }
        }
    }
}


bool time_difference(int last_time)
{
    int current_time = clock();
    int differenza = current_time - last_time;

    if (differenza>= MIN_TIME)
    {
        return true;
    }
    else
    {
        return false;
    }
}


void punteggio(int &mele_mangiate, int& flag_punteggio)
{
    //cambio lo sfondo
    texture = NULL;
    //imposto il colore dello sfondo
    SDL_SetRenderDrawColor(renderer, 163, 236, 236, 0.5);

    //inizializzo ttf per stampare testo sulla finestra
    TTF_Init();

    //imposto un carattere da utilizzare nel testo
    TTF_Font* Sans_bold = TTF_OpenFont("OpenSans-Bold.ttf", 48);
    TTF_Font* Sans_normale = TTF_OpenFont("open-sans/OpenSans-Regular.ttf",24);
    //imposto i colori da utilizzare nel testo
    SDL_Color nero = { 0, 0, 0 };
    SDL_Color rosso = { 255,0,0 };

    //calcolo il punteggio
    int punteggio = mele_mangiate*10;

    //metto in delle string il testo da mostrare
    std::string game_over = "  GAME OVER ";
    std::string stringa_punteggio = "PUNTEGGIO: " + std::to_string(punteggio);
    std::string stringa_fine = "Premere invio per terminare il programma";

    //converto le scritte in suerfici applicabili
    SDL_Surface* superficie_punteggio =TTF_RenderText_Solid(Sans_bold, stringa_punteggio.c_str(), nero);
    SDL_Surface* superficie_game = TTF_RenderText_Solid(Sans_bold, game_over.c_str(), rosso);
    SDL_Surface* superficie_termina = TTF_RenderText_Solid(Sans_normale, stringa_fine.c_str(), nero);

    //imposto la posizione dei tre messaggi
    SDL_Texture* texture_game_over = SDL_CreateTextureFromSurface(renderer, superficie_game);
    SDL_Rect rect_game{};
    TTF_SizeText(Sans_bold, game_over.c_str(),&rect_game.w,&rect_game.h);
    rect_game.x = 150;
    rect_game.y = 200;

    SDL_Texture* texture_messaggio = SDL_CreateTextureFromSurface(renderer, superficie_punteggio);
    SDL_Rect rect_punteggio{};
    TTF_SizeText(Sans_bold, stringa_punteggio.c_str(), &rect_punteggio.w, &rect_punteggio.h);
    rect_punteggio.x = rect_game.x;
    rect_punteggio.y = 200+rect_game.h;

    SDL_Texture* texture_fine = SDL_CreateTextureFromSurface(renderer, superficie_termina);
    SDL_Rect rect_fine{};
    TTF_SizeText(Sans_normale, stringa_fine.c_str(), &rect_fine.w, &rect_fine.h);
    rect_fine.x = 90;
    rect_fine.y = 200+rect_punteggio.y+rect_fine.h;


    //pulisco la renderer e stampo il punteggio
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderCopy(renderer, texture_game_over, NULL, &rect_game);
    SDL_RenderCopy(renderer, texture_messaggio, NULL, &rect_punteggio);
    SDL_RenderCopy(renderer, texture_fine, NULL, &rect_fine);

    SDL_RenderPresent(renderer);

    bool isrunning = true; //variabili per il loop
    SDL_Event event;
    while (isrunning == true) //main loop
    {
        while (SDL_PollEvent(&event) != 0) //finchè la coda di eventi non è 0 fai 
        {
            if (event.type == SDL_QUIT)
            {
                isrunning = false;
            }
            else if(event.type==SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_RETURN)
                {
                    isrunning = false;
                }
            }
        }
    }


    //libero la texture utilizzata
    SDL_DestroyTexture(texture_messaggio);
    SDL_DestroyTexture(texture_game_over);
    SDL_DestroyTexture(texture_fine);
    //chiudo ttf
    TTF_Quit();
}


void update_testa_coda_mela(SDL_Texture* texture, int &x_texture, int &y_texture, SDL_Rect &rect_texture )
{
    //aggiorna le coordinate
    rect_texture.x = x_texture;
    rect_texture.y = y_texture;

    //applica la texture alla renderer
    SDL_RenderCopy(renderer, texture, NULL, &rect_texture);
}

void elimina_texture(SDL_Texture* texture_testa, SDL_Texture* texture_corpo, SDL_Texture* texture_coda, SDL_Texture* texture_mela)
{
    SDL_DestroyTexture(texture_testa);
    SDL_DestroyTexture(texture_mela);
    SDL_DestroyTexture(texture_coda);
    SDL_DestroyTexture(texture_corpo);
}

void end()
{
    texture = NULL;
    renderer = NULL;
    window = NULL;

    //libera la memoria utilizzata
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    //chiudo gli oggetti inizializzati
    SDL_Quit();
}