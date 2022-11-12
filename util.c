#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

SetItem **makeSet() {
  SetItem **set = calloc(1024, sizeof(SetItem));
  return set;
}

char **stringToWords(char *string) {
  char **result = calloc(1024, sizeof(char *));
  char *rest, *token, *stringPtr = string;
  while ((token = strtok_r(stringPtr, " ", &rest))) {
    addCharPtrToArray(result, token);
    stringPtr = rest;
  }
  return result;
}

char *trim(char *str) {
  char *end;
  while (isspace((unsigned char)*str)) str++;
  if (*str == 0) return str;
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) end--;
  end[1] = '\0';
  return str;
}

void addCharPtrToArray(char **array, char *value) {
  for (int i = 0; i < 1024; i++) {
    if (!array[i]) {
      array[i] = value;
      break;
    }
  }
}

void removeCharPtrFromArray(char **array, char *value) {
  for (int i = 0; i < 1024; i++)
    if (array[i] && strcmp(array[i], value) == 0)
      array[i] = NULL;
}

void addToSet(SetItem **set, char *key, char *value) {
  if (inSet(set, key, value)) return;
  for (int i = 0; i < 1024; i++)
    if (set[i] && strcmp(set[i]->key, key) == 0)
      for (int j = 0; j < 1024; j++)
        if (!set[i]->values[j]) {
          set[i]->values[j] = value;
          return;
        }
  for (int i = 0; i < 1024; i++)
    if (!set[i]) {
      set[i] = calloc(1, sizeof(SetItem));
      set[i]->key = key;
      set[i]->values = calloc(1024, sizeof(char *));
      set[i]->values[0] = value;
      return;
    }
}

void addToSetItem(SetItem *setItem, char *value) {
  if (inSetItem(setItem, value)) return;
  for (int i = 0; i < 1024; i++)
    if (!setItem->values[i]) {
      setItem->values[i] = value;
      return;
    }
}

void addAllToSet(SetItem **set, char *key, char **values) {
  if (!values) return;
  for (int i = 0; i < 1024; i++)
    if (values[i])
      addToSet(set, key, values[i]);
}

void addAllToSetItem(SetItem *setItem, char **values) {
  for (int i = 0; i < 1024; i++)
    if (values[i])
      addToSetItem(setItem, values[i]);
}

bool inArray(char **array, char *value) {
  for (int i = 0; i < 1024; i++)
    if (array[i] && strcmp(array[i], value) == 0)
      return true;
  return false;
}

bool inSet(SetItem **set, char *key, char *value) {
  for (int i = 0; i < 1024; i++)
    if (set[i] && strcmp(set[i]->key, key) == 0)
      for (int j = 0; j < 1024; j++)
        if (set[i]->values[j] && strcmp(set[i]->values[j], value) == 0)
          return true;
  return false;
}

bool isKeyInSet(SetItem **set, char *key) {
  for (int i = 0; i < 1024; i++)
    if (set[i] && strcmp(set[i]->key, key) == 0)
      return true;
  return false;
}

bool inSetItem(SetItem *setItem, char *value) {
  for (int i = 0; i < 1024; i++)
    if (setItem->values[i] && strcmp(setItem->values[i], value) == 0)
      return true;
  return false;
}

bool allInSetItem(SetItem *setItem, char **values) {
  for (int i = 0; i < 1024; i++)
    if (values[i] && !inSetItem(setItem, values[i]))
      return false;
  return true;
}

void removeFromSetItem(SetItem *setItem, char *value) {
  for (int i = 0; i < 1024; i++)
    if (setItem->values[i] && strcmp(setItem->values[i], value) == 0)
      setItem->values[i] = NULL;
}

char **findValuesInSet(SetItem **set, char *key) {
  for (int i = 0; i < 1024; i++)
    if (set[i] && strcmp(set[i]->key, key) == 0)
      return set[i]->values;
  return NULL;
}

SetItem *findInSet(SetItem **set, char *key) {
  for (int i = 0; i < 1024; i++)
    if (set[i] && strcmp(set[i]->key, key) == 0)
      return set[i];
  return NULL;
}

int getValuesLength(char **values) {
  for (int i = 0; i < 1024; i++)
    if (!values[i])
      return i;
}

void createSetItem(SetItem **set, char *key) {
  for (int i = 0; i < 1024; i++)
    if (!set[i]) {
      set[i] = calloc(1, sizeof(SetItem));
      set[i]->key = key;
      set[i]->values = calloc(1024, sizeof(char *));
      return;
    }
}

SetItem *copySetItem(SetItem *setItem) {
  SetItem *result = calloc(1, sizeof(SetItem));
  result->values = calloc(1024, sizeof(char *));
  result->key = setItem->key;
  for (int i = 0; i < 1024; i++)
    if (setItem->values[i])
      result->values[i] = setItem->values[i];
  return result;
}

