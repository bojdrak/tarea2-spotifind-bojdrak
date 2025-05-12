#include "tdas/extra.h"
#include "tdas/list.h"
#include "tdas/map.h"
#include "tdas/hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>

#define MAP_SIZE 10000
#define MAX_SONGS_LIST 20 // Cambiar esto a -1 para que no haya limite

typedef struct {
    char *id;
    char *track_name;
    char *album_name;
    char *artist;
    float tempo;
    char *genre;
} Song;


// Voy a usar tres mapas para almacenar la informacion
HashMap* genre_map; 
HashMap* artist_map; 
HashMap* tempo_map;  


// Esta funcion se encarga de almacenar la informacion de las canciones en el mapa correspondiente
void insertInMap(HashMap *map, char *key, Song *song) {
    Pair *pair = searchMap(map, key);
    List *list = pair ? pair->value : NULL;
    if (!list) {
        list = list_create();
        insertMap(map, key, list);
    }
    list_pushBack(list, song);
}

void read_songs() {
  FILE *file = fopen("data/song_dataset_.csv", "r");
  if (file == NULL) {
    perror("Error al abrir el archivo"); 
    return;
  }

  // Lee la primera línea (encabezados) y la ignora
  char **field = read_line_csv(file, ',');
  int k = 0;

  // Lee el resto de las líneas y almacena las canciones
  while ((field = read_line_csv(file, ',')) != NULL) {
    k++; // Contador de canciones

    // Verifica si el campo de la canción es NULL
    Song *new_song = malloc(sizeof(Song));
    if (!new_song) continue;

    // Asigna los valores a la nueva canción
    new_song->id = strdup(field[0]);
    new_song->track_name = strdup(field[4]);
    new_song->artist = strdup(field[2]);
    new_song->album_name = strdup(field[3]);
    new_song->genre = strdup(field[20]);
    new_song->tempo = atof(field[18]);

    // --- GENERO ---
    insertInMap(genre_map, new_song->genre, new_song);

    // --- ARTISTA ---
    insertInMap(artist_map, new_song->artist, new_song);

    // --- TEMPO ---
    char tempo_key[16];
    sprintf(tempo_key, "%d", (int)(new_song->tempo + 0.5)); // Redondeamos el tempo para usarlo como clave
    insertInMap(tempo_map, strdup(tempo_key), new_song);
  }

  // Cierra el archivo y muestra el número de canciones cargadas
  fclose(file);  
  printf("Se cargaron %d canciones\n", k);
}


// Función para buscar canciones por género
void search_genre() {
  char genre[100];
  printf("Ingrese el género: ");
  fgets(genre, 100, stdin);
  genre[strcspn(genre, "\n")] = '\0'; // Quitar salto de línea
  
  // Buscar el género en el mapa
  Pair* pair = searchMap(genre_map, genre);
  if (!pair) {
    printf("No se encontraron canciones del género '%s'\n", genre);
    return;
  }

  // Obtener la lista de canciones asociadas al género
  List* songs = pair->value;

  int counter = 0, showed = 0; // Contador de canciones totales y mostradas
  // Recorrer la lista e imprimir la información de cada canción
  for (Song* per_song = list_first(songs); per_song != NULL; per_song = list_next(songs)) {
    printf("\nTítulo: %s\nArtista: %s\nÁlbum: %s \nGénero: %s\nTempo: %.2f BPM\n", 
          per_song->track_name, 
          per_song->artist,
          per_song->album_name, 
          per_song->genre,
          per_song->tempo);
          
    counter++;
    showed++;
    if (showed == MAX_SONGS_LIST) {
      int opcion;
      printf("\n--- Mostradas %d canciones de %s ---\n", counter, genre);
      printf("1) Mostrar más\n2) Volver al menú\n");
      scanf("%d", &opcion);
      getchar();

      if (opcion != 1) {
          printf("Volviendo al menú...\n");
          return;
      }
      showed = 0;
    }
  }
}

// Función para buscar canciones por artista  
void search_artist() {
  char artist[256];
  printf("Ingrese el nombre del artista: ");
  fgets(artist, 256, stdin);
  artist[strcspn(artist, "\n")] = '\0';

  // Buscar el artista en el mapa
  Pair* pair = searchMap(artist_map, artist);
  if (!pair) {
    printf("No se encontraron canciones del artista '%s'\n", artist);
    return;
  }

  // Obtener la lista de canciones asociadas al artista
  List* songs = pair->value;

  // Recorrer la lista e imprimir la información de cada canción
  for (Song* per_song = list_first(songs); per_song != NULL; per_song = list_next(songs)) {
    printf("\nTítulo: %s\nArtista: %s\nÁlbum: %s \nGénero: %s\nTempo: %.2f BPM\n", 
          per_song->track_name, 
          per_song->artist,
          per_song->album_name, 
          per_song->genre,
          per_song->tempo);
  }
}

// Función para buscar canciones por tempo  
void search_tempo() {
  printf("Seleccione un Numero de categoría para el tempo:\n");
  printf("1) Lentas (menos de 80 BPM)\n");
  printf("2) Moderadas (80 - 120 BPM)\n");
  printf("3) Rápidas (más de 120 BPM)\n");
  
  // Pedir al usuario que seleccione una categoría de tempo, con un numero del 1 al 3
  char speed[100];
  int option;
  scanf("%d", &option);
  getchar();

  float minTempo = 0, maxTempo = 0;
  switch(option) {
      case 1:
          minTempo = 0;
          maxTempo = 79.99;
          strcpy(speed, "lentas");
          break;
      case 2:
          minTempo = 80;
          maxTempo = 120;
          strcpy(speed, "moderadas");
          break;
      case 3:
          minTempo = 120.01;
          maxTempo = 1000; // Un valor alto para no perder datos
          strcpy(speed, "rápidas");
          break;
      default:
          printf("Opción inválida, por favor ingrese un Número\n");
          return;
  }

  
  // Mostrar las canciones que cumplen con el rango de tempo
  int counter = 0, showed = 0; // Contador de canciones totales y mostradas
  Pair* pair = firstMap(tempo_map);
  while (pair != NULL) {
    List* songs = (List*)pair->value;

    // Recorrer la lista de canciones y mostrar las que cumplen con el rango de tempo
    for (Song* per_song = list_first(songs); per_song != NULL; per_song = list_next(songs)) {
        if (per_song->tempo >= minTempo && per_song->tempo <= maxTempo) {
          printf("\nTítulo: %s\nArtista: %s\nÁlbum: %s \nGénero: %s\nTempo: %.2f BPM\n", 
            per_song->track_name, 
            per_song->artist,
            per_song->album_name, 
            per_song->genre,
            per_song->tempo);
          counter++;
          showed++;

          //  Si se han mostrado MAX_SONGS_LIST canciones, preguntar si quiere ver más.
          //  Senti la necesidad de hacer esto porque estamos cargando un gran archivo de canciones
          if (showed == MAX_SONGS_LIST) {
              int opcion;
              printf("\n--- Mostradas %d canciones %s ---\n", counter, speed);
              printf("1) Mostrar más\n2) Volver al menú\n");
              scanf("%d", &opcion);
              getchar();

              if (opcion != 1) {
                  printf("Volviendo al menú...\n");
                  return;
              }
              showed = 0;
          }
        }
      }

      pair = nextMap(tempo_map);
  }

  if (counter == 0) {
      printf("No se encontraron canciones en ese rango de tempo\n");
  } 
} 

// Función para liberar la memoria de una canción
void freeSong(void *data) {
  Song *per_song = (Song *)data;
  free(per_song->id);
  free(per_song->track_name);
  free(per_song->artist);
  free(per_song->album_name);
  free(per_song->genre);
  free(per_song);
}

// Función para liberar la memoria de una lista de canciones
void freeSongList(void *data) {
  List *list = (List *)data;
  list_clean(list, freeSong); // Usamos el callback pa liberar cada Song
  free(list); // Finalmente liberamos la lista misma
}

// Funcion de menu principal
void showMainMenu() {
  clearScreen();
  puts("========================================");
  puts("    Spotifind - Buscador de Canciones   ");
  puts("========================================");

  puts("1) Cargar canciones");
  puts("2) Buscar por genero");
  puts("3) Buscar por artista");
  puts("4) Buscar por tempo");
  puts("5) Salir");
}

int main() {
  setlocale(LC_ALL, ""); // Para que los printf y strings acepten UTF-8

  char option;
  // Inicializa los mapas para almacenar las canciones
  genre_map = createMap(MAP_SIZE);
  artist_map = createMap(MAP_SIZE);
  tempo_map = createMap(MAP_SIZE);

  if (genre_map == NULL || artist_map == NULL || tempo_map == NULL) {
    perror("Error al crear los mapas");
    return EXIT_FAILURE;
  }

  // Abrimos el menu interacitvo e iniciamos el programa
  // El programa se ejecuta hasta que el usuario decida salir
  do {
    showMainMenu();
    printf("Ingrese una opción: ");
    scanf(" %c", &option);
    getchar(); 

    switch (option) {
    case '1':
      read_songs();
      waitForKeyPress(); 
      break;
    case '2':
      search_genre();
      waitForKeyPress(); 
      break;
    case '3':
      search_artist();
      waitForKeyPress(); 
      break;
    case '4':
      search_tempo();
      waitForKeyPress(); 
      break;
    case '5':
      puts("Saliendo de Spotifind...");
      break;
    default:
      puts("Opción inválida. Intente nuevamente.");
    }
    
    
  } while (option != '5');

  // Liberar la memoria de los mapas y las canciones
  freeMap(genre_map, freeSongList);
  freeMap(artist_map, freeSongList);
  freeMap(tempo_map, freeSongList);
  return EXIT_SUCCESS;
}


