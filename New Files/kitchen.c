/** <WRITE NAME AND STUDENT NUMBER HERE> i.e. Ismail Ari, S000001 
 * This is the kitchen simulation code for OzuRest.
 * French chef and 3 students from gastronomy department are preparing delicious meals in here
 * You need to solve their problems.
**/
#include "meal.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define GLOVE_COUNT 3
#define POT_SIZE 3
#define APPRENTICE_COUNT 3
#define MEALS_TO_PREPARE 4
#define REQUIRED_INGREDIENTS 3

struct meal Menu[4] = {
    {"Menemen", {{"Tomato", 3}, {"Onion", 4}, {"Egg", 1}}, 10},
    {"Chicken Pasta", {{"Pasta", 2}, {"Chicken", 5}, {"Curry", 2}}, 8}, 
    {"Beef Steak", {{"Beef", 7}, {"Pepper", 3}, {"Garlic", 2}}, 13}, 
    {"Ali Nazik", {{"Eggplant", 4}, {"Lamb Meat", 4}, {"Yoghurt", 1}}, 10}
}; 

int meal_counter = 0;
int meal_ing_counter = 0;
int n_ingredients_in_pot = 0;

pthread_mutex_t glow_mutex[3];
pthread_mutex_t wear_mutex;

pthread_mutex_t cook_mutex; //Consumer
pthread_mutex_t prepeare_mutex; //Producer
//pthread_mutex_t prepeare_mutex_individual[3];

pthread_mutex_t pick_mutex;

// Define all required mutexes here

void put_gloves(int apprentice_id) {

    printf("Apprentice %d is picking gloves \n", apprentice_id);
    pthread_mutex_lock(&wear_mutex);
    pthread_mutex_lock(&glow_mutex[apprentice_id%3]);
    pthread_mutex_lock(&glow_mutex[(apprentice_id+1)%3]);
    // Implement a mutex lock mechanism for gloves here
    printf("Apprentice %d has picked gloves\n", apprentice_id);
}

void remove_gloves(int apprentice_id) {
    // Implement a mutex unlock mechanism for gloves here
    printf("Apprentice %d has removed gloves\n", apprentice_id);
    pthread_mutex_unlock(&glow_mutex[apprentice_id%3]);
    pthread_mutex_unlock(&glow_mutex[(apprentice_id+1)%3]);
    pthread_mutex_unlock(&wear_mutex);
}

void pick_ingredient(int apprentice_id, int* meal_index, int* ing_index) {
    pthread_mutex_lock(&prepeare_mutex);
    pthread_mutex_unlock(&prepeare_mutex);
    if(meal_counter >=4){
        pthread_exit(NULL);
    }
    put_gloves(apprentice_id);
    // Implement a control mechanism here using mutexes if needed
    //pthread_mutex_lock(&pick_mutex); // This is probably not needed because two apprentices can't take two gloves at the same time anyways
    *meal_index = meal_counter;
    *ing_index = meal_ing_counter;
    printf("Apprentice %d has taken ingredient %s\n", apprentice_id, Menu[*meal_index].ingredients[*ing_index].name);
    // Possible Race condition here, take your precaution
    //meal_counter = meal_counter + 1; //Question: Why do we increase meal_counter in the apprentice function
    meal_ing_counter = meal_ing_counter + 1;
    //pthread_mutex_unlock(&pick_mutex); // This is probably not needed because two apprentices can't take two gloves at the same time anyways
    remove_gloves(apprentice_id);
} 

void prepare_ingredient(int apprentice_id, int meal_index, int ing_index) {
    printf("Apprentice %d is preparing: %s \n", apprentice_id, Menu[meal_index].ingredients[ing_index].name);
    sleep(Menu[meal_index].ingredients[ing_index].time_to_process);
    printf("Apprentice %d is done preparing %s \n", apprentice_id, Menu[meal_index].ingredients[ing_index].name);
}

void put_ingredient(int id, int meal_index, int ing_index) { //TODO: These variables might change unexpectedly
    while(1) { // Question: Why do we need this while loop?
        printf("Apprentince %d is trying to put %s into pot\n", id, Menu[meal_index].ingredients[ing_index].name);
        // Implement a control mechanism here using mutexes to prevent the second problem mentioned in HW file
        pthread_mutex_lock(&prepeare_mutex);
        printf("Apprentince %d has put %s into pot\n", id, Menu[meal_index].ingredients[ing_index].name);
        n_ingredients_in_pot ++;
        if(n_ingredients_in_pot < 3){
            pthread_mutex_unlock(&prepeare_mutex);
            while (n_ingredients_in_pot != 0);
        }
        else{
            pthread_mutex_unlock(&cook_mutex);
        };
        break;
        // Do not forget to break the loop !
    }
}

void help_chef(int apprentice_id) {
    int meal_index, meal_ingredient_index;
    pick_ingredient(apprentice_id, &meal_index, &meal_ingredient_index);
    prepare_ingredient(apprentice_id, meal_index, meal_ingredient_index);
    put_ingredient(apprentice_id, meal_index, meal_ingredient_index);
}

void *apprentice(int *apprentice_id) {
    printf("Im apprentice %d\n", *apprentice_id);
    while(1) {
        help_chef(*apprentice_id);
    }
    pthread_exit(NULL);
}

void *chef() {
    while(1) {
        /** Chef works but you need to implement a control mechanism for him 
         *  to prevent the second problem mentioned in HW file.
         *  As for now, he just cooks without checking the pot.
         */
        pthread_mutex_lock(&cook_mutex);
        printf("Chef is preparing meal %s\n", Menu[meal_counter].name);
        sleep(Menu[meal_counter].time_to_prepare);
        printf("Chef prepared the meal %s\n",  Menu[meal_counter].name);
        meal_ing_counter = 0;
        n_ingredients_in_pot = 0;
        meal_counter = meal_counter + 1;
        pthread_mutex_unlock(&prepeare_mutex);

        sleep(3); // Let the chef rest after preparing the meal
        if(meal_counter == MEALS_TO_PREPARE) // If all meals are prepared chef can go home
            break;
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t apprentice_threads[APPRENTICE_COUNT];
    pthread_t chef_thread;

    int apprentice_ids[APPRENTICE_COUNT] = {0 ,1 ,2};

    for(int i = 0; i<APPRENTICE_COUNT; i++){
        pthread_mutex_init(&glow_mutex[i], NULL);
    }

    pthread_mutex_init(&wear_mutex, NULL);

    // Initialize Glove mutexes here

    void *apprentice(int*);
    void *chef();


    pthread_mutex_lock(&cook_mutex);
    for(int i = 0; i < APPRENTICE_COUNT; i++){
        pthread_create(&apprentice_threads[i], NULL, apprentice, &apprentice_ids[i]);
    }

    pthread_create(&chef_thread, NULL, chef, NULL);
    // Initialize threads here

    // Tell the main thread to wait other threads to complete
    for (size_t i = 0; i < APPRENTICE_COUNT; i++)
        pthread_join(apprentice_threads[i],NULL);

    pthread_join(chef_thread, NULL);

    return 0;
}
