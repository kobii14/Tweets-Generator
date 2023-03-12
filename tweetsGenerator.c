#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define POINT "."
#define TWEET "Tweet %d: %s"
#define ALLOCATION_ERROR "Allocation failure: bad malloc"
#define USAGE_ERROR "Usage: <seed> <tweets num> <path> <words to read>"
#define FILE_ERROR "Error: THE FILE DOESN'T EXIST"
#define GOOD_ARGC_4 4
#define GOOD_ARGC_5 5
#define MAX_WORDS_IN_SENTENCE_GENERATION 20
#define MAX_WORD_LENGTH 100
#define MAX_SENTENCE_LENGTH 1000

typedef struct WordStruct {
    char *word;
    int counter;
    int prob_diversity;
    struct WordProbability *prob_list;
} WordStruct;

typedef struct WordProbability {
    struct WordStruct *word_struct_ptr;
    int rep_counter;

} WordProbability;

/************ LINKED LIST ************/
typedef struct Node {
    WordStruct *data;
    struct Node *next;
} Node;

typedef struct LinkList {
    Node *first;
    Node *last;
    int size;
} LinkList;

/**
 * Add data to new node at the end of the given link list.
 * @param link_list Link list to add data to
 * @param data pointer to dynamically allocated data
 * @return 0 on success, 1 otherwise
 */
int add(LinkList *link_list, WordStruct *data)
{
  Node *new_node = malloc(sizeof(Node));
  if (new_node == NULL)
    {
      return 1;
    }
  *new_node = (Node){data, NULL};
  if (link_list->first == NULL)
    {
      link_list->first = new_node;
      link_list->last = new_node;
    }
  else
    {
      link_list->last->next = new_node;
      link_list->last = new_node;
    }
  link_list->size++;
  return 0;
}
/*************************************/
/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number)
{
  int rand_num = rand() % max_number;
  return rand_num;
}
/**
 * Choose randomly the next word from the given dictionary, drawn uniformly.
 * The function won't return a word that end's in full stop '.' (Nekuda).
 * @param dictionary Dictionary to choose a word from
 * @return WordStruct of the chosen word
 */
WordStruct *get_first_random_word(LinkList *dictionary)
{
  int check_point = 0;
  Node *temp_node = dictionary->first;
  do
    {
      temp_node = dictionary->first;
      int rand_num = get_random_number(dictionary->size);
      for (int i =0; i<rand_num ; i++)
        {
          temp_node = temp_node->next;
        }
      check_point = strcmp(&temp_node->data->word
      [strlen(temp_node->data->word) - 1], POINT);
    }
  while (check_point == 0);
  return temp_node->data;
}
/**
 * Choose randomly the next word. Depend on it's occurrence frequency
 * in word_struct_ptr->WordProbability.
 * @param word_struct_ptr WordStruct to choose from
 * @return WordStruct of the chosen word
 */
WordStruct *get_next_random_word(WordStruct *word_struct_ptr)
{
  int total_prob = 0;
  for (int i = 0; i < word_struct_ptr->prob_diversity; ++i)
    {
      total_prob += word_struct_ptr->prob_list[i].rep_counter;
    }
  int rand_num = get_random_number(total_prob);
  int count_probs = 0;
  for (int i = 0; i < word_struct_ptr->prob_diversity; ++i)
    {
      count_probs += word_struct_ptr->prob_list[i].rep_counter;
      if (rand_num < count_probs)
        {
          return word_struct_ptr->prob_list[i].word_struct_ptr;
        }
    }
  return NULL;
}
/**
 * Receive dictionary, generate and print to stdout random sentence out of it.
 * The sentence most have at least 2 words in it.
 * @param dictionary Dictionary to use
 * @return Amount of words in printed sentence
 */
int generate_sentence(LinkList *dictionary)
{
  WordStruct *first_word = get_first_random_word(dictionary);
  int word_counter = 1;
  static int number = 1;
  int check_point = 0;
  printf(TWEET, number++, first_word->word);
  do
    {
      WordStruct *next_word = get_next_random_word(first_word);
      printf(" %s", next_word->word);
      first_word = next_word;
      check_point = strcmp(&next_word->word[strlen(next_word->word) - 1],
                           POINT);
      word_counter++;
    }
  while (check_point != 0 && word_counter < MAX_WORDS_IN_SENTENCE_GENERATION);
  return word_counter;
}
/**
 * Gets 2 WordStructs. If second_word in first_word's prob_list,
 * update the existing probability value.
 * Otherwise, add the second word to the prob_list of the first word.
 * @param first_word
 * @param second_word
 * @return 0 if already in list, 1 otherwise.
 */
int add_word_to_probability_list(WordStruct *first_word,
                                 WordStruct *second_word)
{
  if (first_word->prob_list == NULL)
    {
      WordProbability *prob_word = malloc(sizeof(WordProbability));
      if (prob_word == NULL)
        {
          printf(ALLOCATION_ERROR);
          exit(EXIT_FAILURE);
        }
      first_word->prob_list = prob_word;
      first_word->prob_list->word_struct_ptr = second_word;
      first_word->prob_list->rep_counter = 1;
      first_word->prob_diversity = 1;
      return 1;
    }
  int i = 0;
  while (i < first_word->prob_diversity)
    {
      int compare = strcmp(first_word->prob_list[i].word_struct_ptr->word,
                           second_word->word);
      if (compare == 0)
        {
          first_word->prob_list[i].rep_counter ++;
          return 0;
        }
      i ++;
    }
  first_word->prob_list = realloc(first_word->prob_list,
                                  (first_word->prob_diversity + 1) *
                                  sizeof(WordProbability));
  if (first_word->prob_list == NULL)
    {
      printf(ALLOCATION_ERROR);
      exit(EXIT_FAILURE);
    }
  first_word->prob_list[first_word->prob_diversity].word_struct_ptr
      = second_word;
  first_word->prob_list[first_word->prob_diversity].rep_counter = 1;
  first_word->prob_diversity++;
  return 1;
}
//get a pointer to LinkedList dictionary and a pointer to chat for the data of
// a specific word in the dictionary. Return a pointer to the wordstruct
// if it exists in the dictionary, NULL otherwise.
WordStruct* in_dict_check(LinkList *dictionary, char *word_data)
{
  if (dictionary->size == 0)
    {
      return NULL;
    }
  Node *temp_node = dictionary->first;
  while (temp_node != NULL)
    {
      char *a[MAX_WORD_LENGTH];
      *a = temp_node->data->word;
      char *b[MAX_WORD_LENGTH];
      *b = word_data;
      int compare = strcmp(*a, *b);
      if (compare == 0)
        {
          temp_node->data->counter += 1;
          return temp_node->data;
        }
      temp_node = temp_node->next;
    }
  return NULL;
}
/**
 * Read word from the given file. Add every unique word to the dictionary.
 * Also, at every iteration, update the prob_list of the previous word with
 * the value of the current word.
 * @param fp File pointer
 * @param words_to_read Number of words to read from file.
 *                      If value is bigger than the file's word count,
 *                      or if words_to_read == -1 than read entire file.
 * @param dictionary Empty dictionary to fill
 */
void fill_dictionary(FILE *fp, int words_to_read, LinkList *dictionary)
{
  char row_text[MAX_SENTENCE_LENGTH] = {0};
  WordStruct *prev_word = NULL;
  int word_counter = 0;
  char *new_word = NULL;
  char *token;
  while (fgets (row_text, MAX_SENTENCE_LENGTH, fp) != NULL && (word_counter
                                                               < words_to_read
                                                               || words_to_read
                                                                  < 0))
    {
      token = strtok (row_text, " \n");
      while (token != NULL)
        {
          new_word = malloc (strlen (token) + 1);
          strcpy (new_word, token);
          word_counter++;
          WordStruct *pointer_to_ws = NULL;
          pointer_to_ws = (in_dict_check (dictionary, new_word));
          if (pointer_to_ws != NULL)
            {
              if (prev_word != NULL)
                {
                  add_word_to_probability_list (prev_word, pointer_to_ws);
                }
              prev_word = pointer_to_ws;
              int check_point = strcmp(&new_word[strlen(new_word) - 1], POINT);
              if (check_point == 0)
                {
                  prev_word = NULL;
                }
              free(new_word);
              token = strtok (NULL, " \n");
              continue;
            }
          WordStruct *new_word_struct = malloc (sizeof (WordStruct));
          if (new_word_struct == NULL)
            {
              fclose (fp);
              printf (ALLOCATION_ERROR);
              exit (EXIT_FAILURE);
            }
          new_word_struct->word = new_word;
          new_word_struct->counter = 1;
          new_word_struct->prob_list = NULL;
          new_word_struct->prob_diversity = 0;
          int add_check = add (dictionary, new_word_struct);
          if (add_check == 1)
            {
              fclose (fp);
              printf (ALLOCATION_ERROR);
              exit (EXIT_FAILURE);
            }
          if (prev_word != NULL)
            {
              add_word_to_probability_list (prev_word, new_word_struct);
            }
          prev_word = new_word_struct;
          int check_point = strcmp (&new_word[strlen (new_word) - 1], POINT);
          if (check_point == 0)
            {
              prev_word = NULL;
            }
          token = strtok (NULL, " \n");
        }
    }
}
/**
 * Free the given dictionary and all of it's content from memory.
 * @param dictionary Dictionary to free
 */
void free_dictionary(LinkList *dictionary)
{
  int i = 0;
  Node *temp_node = dictionary->first;
  Node *next;
  for (i = 0; i < dictionary->size; i++)
    {
      free(temp_node->data->prob_list);
      temp_node->data->prob_list = NULL;
      next = temp_node->next;
      free(temp_node->data->word);
      temp_node->data->word = NULL;
      temp_node->next = NULL;
      free(temp_node->data);
      temp_node->data = NULL;
      free(temp_node);
      temp_node = NULL;
      temp_node = next;
    }
  free(dictionary);
  dictionary = NULL;
}
//gets a *char for a path of a file and returns 1 if the file doesn't exist,
// 0 otherwise
int file_existence (char *path)
{
  FILE *input_file = NULL;
  if ((input_file = fopen (path, "r")))
    {
      fclose (input_file);
      return 1;
    }
  return 0;
}

/**
 * @param argc
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 *             3) Path to file
 *             4) Optional - Number of words to read
 */
int main(int argc, char *argv[])
{
  //check for valid argc number
  if (argc != GOOD_ARGC_4 && argc != GOOD_ARGC_5)
    {
      printf (USAGE_ERROR);
      exit (EXIT_FAILURE);
    }
  int seed_num;
  int tweets_num;
  //check for good sscanf
  int check_1 = sscanf (argv[1], "%d", &seed_num);
  if (check_1 != 1)
    {
      printf (USAGE_ERROR);
      exit (EXIT_FAILURE);
    }
  //check for good sscanf
  int check_2 = sscanf (argv[2], "%d", &tweets_num);
  if (check_2 != 1)
    {
      printf(USAGE_ERROR);
      exit (EXIT_FAILURE);
    }
    //check for file existence
  int check_file = file_existence (argv[3]);
  if (check_file == 0)
    {
      printf(FILE_ERROR);
      exit (EXIT_FAILURE);
    }
  FILE *corpus_text = NULL;
  corpus_text = fopen (argv[3], "r");
  LinkList *new_dict = malloc (sizeof (LinkList));
  //check the memory allocation
  if (new_dict == NULL)
    {
      printf(ALLOCATION_ERROR);
      fclose(corpus_text);
      exit(EXIT_FAILURE);
    }
    //initialize dictionary values
  new_dict->first = NULL;
  new_dict->last = NULL;
  new_dict->size = 0;
  if (argc == GOOD_ARGC_4)
    {
      fill_dictionary (corpus_text, -1, new_dict);
    }
  else if (argc == GOOD_ARGC_5)
    {
      int words_to_read = 0;
      int check_3 = sscanf (argv[4], "%d", &words_to_read);
      if (check_3 != 1)
        {
          printf (USAGE_ERROR);
          exit (EXIT_FAILURE);
        }
      fill_dictionary (corpus_text, words_to_read, new_dict);
    }
  srand (*argv[1]);
  int i = 0;
  //generate sentences
  for (i = 0; i < tweets_num; i++)
    {
      generate_sentence(new_dict);
      printf("\n");
    }
  free_dictionary(new_dict);
  fclose (corpus_text);
}
