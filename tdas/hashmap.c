#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "hashmap.h"


typedef struct HashMap HashMap;
int enlarge_called=0;

struct HashMap {
    Pair ** buckets;
    long size; //cantidad de datos/pairs en la tabla
    long capacity; //capacidad de la tabla
    long current; //indice del ultimo dato accedido
};

Pair * createPair( char * key,  void * value) {
    Pair * new = (Pair *)malloc(sizeof(Pair));
    new->key = key;
    new->value = value;
    return new;
}

long hash( char * key, long capacity) {
    unsigned long hash = 0;
     char * ptr;
    for (ptr = key; *ptr != '\0'; ptr++) {
        hash += hash*32 + tolower(*ptr);
    }
    return hash%capacity;
}

int is_equal(void* key1, void* key2){
    if(key1==NULL || key2==NULL) return 0;
    if(strcmp((char*)key1,(char*)key2) == 0) return 1;
    return 0;
}


void insertMap(HashMap * map, char * key, void * value) {
    if ((float)(map->size + 1) / map->capacity > 0.75) {
        enlarge(map);
    }

    long index = hash(key, map->capacity);

    while (map->buckets[index] != NULL && map->buckets[index]->key != NULL) {
        if (is_equal(map->buckets[index]->key, key)) {
            map->buckets[index]->value = value;
            return;
        }
        index = (index + 1) % map->capacity;
    }

    map->buckets[index] = createPair(key, value);
    map->size++;
    map->current = index;
}

void enlarge(HashMap * map) {
    enlarge_called = 1; // no borrar (testing)
    long old_capacity = map->capacity;
    Pair **old_buckets = map->buckets;

    long new_capacity = old_capacity * 2;
    Pair **new_buckets = (Pair **)malloc(sizeof(Pair *) * new_capacity);
    for (long i = 0; i < new_capacity; i++)
        new_buckets[i] = NULL;

    map->buckets = new_buckets;
    map->capacity = new_capacity;
    map->size = 0;

    for (long i = 0; i < old_capacity; i++) {
        if (old_buckets[i] != NULL && old_buckets[i]->key != NULL) {
            insertMap(map, old_buckets[i]->key, old_buckets[i]->value);
            free(old_buckets[i]);
        }
    }

    free(old_buckets);
}


HashMap * createMap(long capacity) {
    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    if (map == NULL) return NULL;

    map->buckets = (Pair **)malloc(sizeof(Pair *) * capacity);
    if (map->buckets == NULL) {
        free(map);
        return NULL;
    }

    for (long i = 0; i < capacity; i++) {
        map->buckets[i] = NULL;
    }

    map->capacity = capacity;
    map->size = 0;
    map->current = -1;

    return map;
}

void eraseMap(HashMap * map,  char * key) {    
    long index = hash(key, map->capacity);
    long start = index;

    while (map->buckets[index] != NULL) {
        if (map->buckets[index]->key != NULL && is_equal(map->buckets[index]->key, key)) {
            map->buckets[index]->key = NULL;
            map->size--;
            return;
        }
        index = (index + 1) % map->capacity;
        if (index == start) break;
    }
}

Pair * searchMap(HashMap * map,  char * key) {   
    long index = hash(key, map->capacity);
    long start = index;

    while (map->buckets[index] != NULL) {
        if (map->buckets[index]->key != NULL && is_equal(map->buckets[index]->key, key)) {
            map->current = index;
            return map->buckets[index];
        }
        index = (index + 1) % map->capacity;
        if (index == start) break;
    }

    return NULL;
}

Pair * firstMap(HashMap * map) {
    for (long i = 0; i < map->capacity; i++) {
        if (map->buckets[i] != NULL && map->buckets[i]->key != NULL) {
            map->current = i;
            return map->buckets[i];
        }
    }
    return NULL;
}

Pair * nextMap(HashMap * map) {
    for (long i = map->current + 1; i < map->capacity; i++) {
        if (map->buckets[i] != NULL && map->buckets[i]->key != NULL) {
            map->current = i;
            return map->buckets[i];
        }
    }
    return NULL;
}


void freeMap(HashMap * map, void (*freeValue)(void *)) {
    for (long i = 0; i < map->capacity; i++) {
        if (map->buckets[i] != NULL) {
            Pair *pair = map->buckets[i];

            // Libera la clave (si fue strdup por ejemplo)
            free(pair->key);

            // Libera el valor usando función custom
            if (freeValue != NULL && pair->value != NULL)
                freeValue(pair->value);

            // Libera el par
            free(pair);
        }
    }
    free(map->buckets);
    free(map);
}



