#include "lr1.h"
#include "grammar.h"
#include "util.h"
#include "clex/clex.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

void closure(Grammar *grammar, LR1Item **items);

LR1Parser *makeParser() {
  LR1Parser *parser = calloc(1, sizeof(LR1Parser));
  parser->collection = calloc(1024, sizeof(LR1State *));
  parser->goToTable = calloc(1024, sizeof(GoToNode **));
  parser->actionTable = calloc(1024, sizeof(ActionNode **));
  return parser;
}

LR1Item *makeLR1Item(char *left, char **right, int dot, char **lookaheads) {
  LR1Item *item = calloc(1, sizeof(LR1Item));
  item->left = left;
  item->right = right;
  item->dot = dot;
  item->lookaheads = lookaheads;
  return item;
}

LR1State *makeLR1State(Grammar *grammar, LR1Item **items) {
  LR1State *state = calloc(1, sizeof(LR1State));
  state->items = items;
  state->transitions = calloc(1024, sizeof(LR1Transition *));
  closure(grammar, state->items);
  return state;
}

LR1Transition *makeTransition(char *value, LR1State *state) {
  LR1Transition *transition = calloc(1, sizeof(LR1Transition));
  transition->value = value;
  transition->state = state;
  return transition;
}

GoToNode *makeGoToNode(char *nonterminal, int state) {
  GoToNode *node = calloc(1, sizeof(GoToNode));
  node->nonterminal = nonterminal;
  node->state = state;
  return node;
}

ActionNode *makeActionNode(char *terminal, ActionType type, int operand) {
  ActionNode *node = calloc(1, sizeof(ActionNode));
  Action *action = calloc(1, sizeof(Action));
  action->type = type;
  action->operand = operand;
  node->action = action;
  node->terminal = terminal;
  return node;
}

bool isNonTerminal(Grammar *grammar, char *value) {
  for (int i = 0; i < 1024; i++)
    if (grammar->nonterminals[i] && strcmp(grammar->nonterminals[i], value) == 0)
      return true;
  return false;
}

char **computeFirstByIndex(Grammar *grammar, char **strings, int index) {
  char **result = calloc(1024, sizeof(char *));
  if (index == getValuesLength(strings))
    return result;
  if (inArray(grammar->terminals, strings[index]) || strcmp(strings[index], "#") == 0) {
    addCharPtrToArray(result, strings[index]);
    return result;
  }
  if (inArray(grammar->nonterminals, strings[index]))
    addAllCharPtrToArrayUnique(result, findValuesInSet(grammar->first, strings[index]));
  if (inArray(result, "#"))
    if (index != getValuesLength(strings) - 1) {
      removeCharPtrFromArray(result, "#");
      addAllCharPtrToArrayUnique(result, computeFirstByIndex(grammar, strings, index + 1));
    }
  return result;
}

bool equalLR0(LR1Item *a, LR1Item *b) {
  if (a->dot != b->dot)
    return false;
  if (strcmp(a->left, b->left) != 0)
    return false;
  if (!isArrayEqual(a->right, b->right))
    return false;
  return true;
}

void addItem(LR1Item **items, LR1Item *item) {
  for (int i = 0; i < 1024; i++)
    if (!items[i]) {
      items[i] = item;
      break;
    }
}

void addState(LR1State **states, LR1State *state) {
  for (int i = 0; i < 1024; i++)
    if (!states[i]) {
      states[i] = state;
      break;
    }
}

void addTransition(LR1Transition **transitions, LR1Transition *transition) {
  for (int i = 0; i < 1024; i++)
    if (!transitions[i]) {
      transitions[i] = transition;
      break;
    }
}

void closure(Grammar *grammar, LR1Item **items) {
  bool changed = false;
  do {
    changed = false;
    for (int i = 0; i < 1024; i++) {
      if (items[i]) {
        if (items[i]->dot != getValuesLength(items[i]->right) && isNonTerminal(grammar, items[i]->right[items[i]->dot])) {
          char **lookaheads = calloc(1024, sizeof(char *));
          if (items[i]->dot == getValuesLength(items[i]->right) - 1) {
            addAllCharPtrToArrayUnique(lookaheads, items[i]->lookaheads);
          } else {
            char **firstSet = computeFirstByIndex(grammar, items[i]->right, items[i]->dot + 1);
            if (inArray(firstSet, "#")) {
              removeCharPtrFromArray(firstSet, "#");
              addAllCharPtrToArrayUnique(firstSet, items[i]->lookaheads);
            }
            addAllCharPtrToArrayUnique(lookaheads, firstSet);
          }
          for (int j = 0; j < 1024; j++) {
            if (grammar->rules[j] && strcmp(grammar->rules[j]->left, items[i]->right[items[i]->dot]) == 0) {
              char **right = grammar->rules[j]->right;
              int finish = 0;
              if (getValuesLength(right) == 1 && strcmp(right[0], "#") == 0)
                finish = 1;
              char **newLookaheads = copyCharArray(lookaheads);
              LR1Item *newItem = makeLR1Item(grammar->rules[j]->left, right, finish, newLookaheads);
              bool found = false;
              for (int x = 0; x < 1024; x++) {
                if (items[x]) {
                  if (equalLR0(items[x], newItem)) {
                    if (!arrayContainsAll(items[x]->lookaheads, newLookaheads)) {
                      addAllCharPtrToArrayUnique(items[x]->lookaheads, newLookaheads);
                      changed = true;
                    }
                    found = true;
                    break;
                  }
                }
              }
              if (!found) {
                addItem(items, newItem);
                changed = true;
              }
            }
          }
          if (changed)
            break;
        }
      }
    }
  } while (changed);
}

bool inArrayLR1Item(LR1Item **array, LR1Item *item) {
  for (int i = 0; i < 1024; i++)
    if (array[i] && array[i]->dot == item->dot && strcmp(array[i]->left, item->left) == 0 && isArrayEqual(array[i]->right, item->right) && isArrayEqual(array[i]->lookaheads, item->lookaheads))
      return true;
  return false;
}

bool arrayContainsAllLR1Items(LR1Item **a, LR1Item **b) {
  for (int i = 0; i < 1024; i++)
    if (b[i] && !inArrayLR1Item(a, b[i]))
      return false;
  return true;
}

void createCollection(LR1Parser *parser, Grammar *grammar) {
  LR1Item **startItems = calloc(1024, sizeof(LR1Item *));
  char **startLookahead = calloc(1024, sizeof(char *));
  startLookahead[0] = "$";
  Rule *firstRule = grammar->rules[0];
  startItems[0] = makeLR1Item(firstRule->left, firstRule->right, 0, startLookahead);
  LR1State *startState = makeLR1State(grammar, startItems);
  parser->collection[0] = startState;
  for (int i = 0; i < 1024; i++) {
    if (parser->collection[i]) {
      char **strings = calloc(1024, sizeof(char *));
      for (int j = 0; j < 1024; j++)
        if (parser->collection[i]->items[j])
          if (parser->collection[i]->items[j]->right[parser->collection[i]->items[j]->dot])
            addCharPtrToArray(strings, parser->collection[i]->items[j]->right[parser->collection[i]->items[j]->dot]);
      for (int j = 0; j < 1024; j++) {
        if (strings[j]) {
          LR1Item **items = calloc(1024, sizeof(LR1Item *));
          for (int x = 0; x < 1024; x++) {
            if (parser->collection[i]->items[x]) {
              if (parser->collection[i]->items[x]->right[parser->collection[i]->items[x]->dot] && strcmp(parser->collection[i]->items[x]->right[parser->collection[i]->items[x]->dot], strings[j]) == 0) {
                addItem(items, makeLR1Item(parser->collection[i]->items[x]->left, parser->collection[i]->items[x]->right, parser->collection[i]->items[x]->dot + 1, parser->collection[i]->items[x]->lookaheads));
              }
            }
          }
          LR1State *state = makeLR1State(grammar, items);
          bool exists = false;
          for (int x = 0; x < 1024; x++) {
            if (parser->collection[x]) {
              if (arrayContainsAllLR1Items(parser->collection[x]->items, state->items) && arrayContainsAllLR1Items(state->items, parser->collection[x]->items)) {
                exists = true;
                addTransition(parser->collection[i]->transitions, makeTransition(strings[j], parser->collection[x]));
              }
            }
          }
          if (!exists) {
            addState(parser->collection, state);
            addTransition(parser->collection[i]->transitions, makeTransition(strings[j], state));
          }
        }
      }
    }
  }
}

void addGoToNode(GoToNode **table, GoToNode *node) {
  for (int i = 0; i < 1024; i++)
    if (!table[i]) {
      table[i] = node;
      break;
    }
}

void addActionNode(ActionNode **table, ActionNode *node) {
  for (int i = 0; i < 1024; i++)
    if (!table[i]) {
      table[i] = node;
      break;
    }
}

int getIndexOfState(LR1State **collection, LR1State *state) {
  for (int i = 0; i < 1024; i++)
    if (collection[i] && collection[i] == state)
      return i;
  return -1;
}

void createGoToTable(LR1Parser *parser, Grammar *grammar) {
  for (int i = 0; i < 1024; i++) {
    if (parser->collection[i]) {
      parser->goToTable[i] = calloc(1024, sizeof(GoToNode *));
    }
  }
  for (int i = 0; i < 1024; i++) {
    if (parser->collection[i]) {
      for (int j = 0; j < 1024; j++) {
        if (parser->collection[i]->transitions[j]) {
          if (isNonTerminal(grammar, parser->collection[i]->transitions[j]->value)) {
            addGoToNode(parser->goToTable[i], makeGoToNode(parser->collection[i]->transitions[j]->value, getIndexOfState(parser->collection, parser->collection[i]->transitions[j]->state)));
          }
        }
      }
    }
  }
}

int getRuleIndex(Grammar *grammar, char *left, char **right) {
  for (int i = 0; i < 1024; i++)
    if (grammar->rules[i])
      if (strcmp(grammar->rules[i]->left, left) == 0 && isArrayEqual(grammar->rules[i]->right, right))
        return i;
  return -1;
}

bool isTerminalInActionTable(ActionNode **table, char *terminal) {
  for (int i = 0; i < 1024; i++)
    if (table[i] && strcmp(table[i]->terminal, terminal) == 0)
      return true;
  return false;
}

void createActionTable(LR1Parser *parser, Grammar *grammar) {
  for (int i = 0; i < 1024; i++) {
    if (parser->goToTable[i]) {
      parser->actionTable[i] = calloc(1024, sizeof(ActionNode *));
    }
  }
  for (int i = 0; i < 1024; i++)
    if (parser->collection[i])
      for (int j = 0; j < 1024; j++)
        if (parser->collection[i]->transitions[j])
          if (!isNonTerminal(grammar, parser->collection[i]->transitions[j]->value))
            addActionNode(parser->actionTable[i], makeActionNode(parser->collection[i]->transitions[j]->value, SHIFT, getIndexOfState(parser->collection, parser->collection[i]->transitions[j]->state)));
  for (int i = 0; i < 1024; i++) {
    if (parser->collection[i]) {
      for (int j = 0; j < 1024; j++) {
        if (parser->collection[i]->items[j]) {
          LR1Item *item = parser->collection[i]->items[j];
          if (item->dot == getValuesLength(item->right)) {
            if (strcmp(item->left, "cparseStart") == 0) {
              addActionNode(parser->actionTable[i], makeActionNode("$", ACCEPT, 0));
            } else {
              int index = getRuleIndex(grammar, item->left, item->right);
              for (int x = 0; x < 1024; x++) {
                if (item->lookaheads[x]) {
                  if (isTerminalInActionTable(parser->actionTable[i], item->lookaheads[x])) {
                    fprintf(stderr, "Conflict found\n");
                  } else {
                    addActionNode(parser->actionTable[i], makeActionNode(item->lookaheads[x], REDUCE, index));
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

LR1Parser *createLR1Parser(Grammar *grammar, const char * const *tokenKindStr) {
  LR1Parser *parser = makeParser();
  parser->grammar = grammar;
  parser->tokenKindStr = tokenKindStr;
  createCollection(parser, grammar);
  createGoToTable(parser, grammar);
  createActionTable(parser, grammar);
  return parser;
}

char *getCharPtrArrayAsString(char **array) {
  char *result = calloc(10000, sizeof(char));
  for (int i = 0; i < 1024; i++)
    if (array[i])
      sprintf(result + strlen(result), "%s, ", array[i]);
  return result;
}

char *getLR1ParserAsString(LR1Parser *parser) {
  char *result = calloc(100000, sizeof(char));
  sprintf(result, "Collection:\n");
  for (int i = 0; i < 1024; i++) {
    if (parser->collection[i]) {
      sprintf(result + strlen(result), "State %d:\n", i);
      sprintf(result + strlen(result), "Items:\n");
      for (int j = 0; j < 1024; j++) {
        if (parser->collection[i]->items[j]) {
          sprintf(result + strlen(result), "%s -> %s\n", parser->collection[i]->items[j]->left, getCharPtrArrayAsString(parser->collection[i]->items[j]->right));
          sprintf(result + strlen(result), "Dot: %d\n", parser->collection[i]->items[j]->dot);
          sprintf(result + strlen(result), "Lookaheads: %s\n", getCharPtrArrayAsString(parser->collection[i]->items[j]->lookaheads));
        }
      }
      sprintf(result + strlen(result), "Transitions:\n");
      for (int j = 0; j < 1024; j++) {
        if (parser->collection[i]->transitions[j]) {
          sprintf(result + strlen(result), "%s -> %d\n", parser->collection[i]->transitions[j]->value, getIndexOfState(parser->collection, parser->collection[i]->transitions[j]->state));
        }
      }
    }
  }
  sprintf(result + strlen(result), "GoTo table:\n");
  for (int i = 0; i < 1024; i++) {
    if (parser->goToTable[i]) {
      for (int j = 0; j < 1024; j++) {
        if (parser->goToTable[i][j]) {
          sprintf(result + strlen(result), "%d %s -> %d\n", i, parser->goToTable[i][j]->nonterminal, parser->goToTable[i][j]->state);
        }
      }
    }
  }
  sprintf(result + strlen(result), "\n");
  sprintf(result + strlen(result), "Action table:\n");
  for (int i = 0; i < 1024; i++) {
    if (parser->actionTable[i]) {
      for (int j = 0; j < 1024; j++) {
        if (parser->actionTable[i][j]) {
          sprintf(result + strlen(result), "%d %s -> %d %d\n", i, parser->actionTable[i][j]->terminal, parser->actionTable[i][j]->action->type, parser->actionTable[i][j]->action->operand);
        }
      }
    }
  }
  return result;
}

char *peek(char **stack) {
  char *result;
  for (int i = 0; i < 1024; i++)
    if (stack[i]) result = stack[i];
    else return result;
  return result;
}

char *pop(char **stack) {
  for (int i = 1; i < 1024; i++)
    if (!stack[i] && stack[i - 1]) {
      char *value = stack[i - 1];
      stack[i - 1] = NULL;
      return value;
    }
  return NULL;
}

ActionNode *getAction(ActionNode **table, const char *terminal) {
  for (int i = 0; i < 1024; i++)
    if (table[i])
      if (strcmp(table[i]->terminal, terminal) == 0)
        return table[i];
  return NULL;
}

int getGoToState(GoToNode **table, char *nonterminal) {
  for (int i = 0; i < 1024; i++)
    if (table[i])
      if (strcmp(table[i]->nonterminal, nonterminal) == 0)
        return table[i]->state;
  return -1;
}

char *intToString(int number) {
  int n = log10(number) + 1;
  int i;
  char *numberArray = calloc(n, sizeof(char));
  for (i = n - 1; i >= 0; --i, number /= 10)
    numberArray[i] = (number % 10) + '0';
  return numberArray;
}

bool accept(LR1Parser *parser, char *input) {
  clexInit(input);
  Token token;
  char **stack = calloc(1024, sizeof(char *));
  addCharPtrToArray(stack, "0");
  bool lexNext = true;
  while (true) {
    if (lexNext)
      token = clex();
    lexNext = true;
    int state = atoi(peek(stack));
    const char *nextInput = token.kind ? parser->tokenKindStr[token.kind] : "$"; // TODO: Add $ only once
    ActionNode *action = getAction(parser->actionTable[state], nextInput);
    if (action == NULL) {
      return false;
    } else if (action->action->type == SHIFT) {
      addCharPtrToArray(stack, (char *)nextInput);
      addCharPtrToArray(stack, intToString(action->action->operand));
    } else if (action->action->type == REDUCE) {
      Rule *rule = parser->grammar->rules[action->action->operand];
      for (int i = 0; i < 2 * getValuesLength(rule->right); i++)
        pop(stack);
      int nextState = atoi(peek(stack));
      addCharPtrToArray(stack, rule->left);
      int goToState = getGoToState(parser->goToTable[nextState], rule->left);
      addCharPtrToArray(stack, intToString(goToState));
      lexNext = false;
    } else if (action->action->type == ACCEPT) {
      return true;
    }
  }
  return false;
}

bool isNode(Grammar *grammar, char *value) {
  return inArray(grammar->terminals, value) || inArray(grammar->nonterminals, value);
}

ParseTreeNode *makeParseTreeNode(char *value) {
  ParseTreeNode *node = calloc(1, sizeof(ParseTreeNode));
  node->children = calloc(1024, sizeof(char *));
  node->value = value;
  return node;
}

ParseTreeNode *parse(LR1Parser *parser, char *input) {
  clexInit(input);
  Token token;
  char **stack = calloc(1024, sizeof(char *));
  addCharPtrToArray(stack, "0");
  bool lexNext = true;
  ParseTreeNode *root;
  while (true) {
    if (lexNext)
      token = clex();
    lexNext = true;
    int state = atoi(peek(stack));
    const char *nextInput = token.kind ? parser->tokenKindStr[token.kind] : "$"; // TODO: Add $ only once
    ActionNode *action = getAction(parser->actionTable[state], nextInput);
    if (action == NULL) {
      return NULL;
    } else if (action->action->type == SHIFT) {
      addCharPtrToArray(stack, (char *)nextInput);
      addCharPtrToArray(stack, intToString(action->action->operand));
    } else if (action->action->type == REDUCE) {
      Rule *rule = parser->grammar->rules[action->action->operand];
      root = makeParseTreeNode(rule->left);
      for (int i = 0; i < 2 * getValuesLength(rule->right); i++) {
        char *popped = pop(stack);
        if (isNode(parser->grammar, popped))
          for (int i = 0; i < 1024; i++)
            if (!root->children[i]) {
              root->children[i] = makeParseTreeNode(popped);
              break;
            }
      }
      int nextState = atoi(peek(stack));
      addCharPtrToArray(stack, rule->left);
      int goToState = getGoToState(parser->goToTable[nextState], rule->left);
      addCharPtrToArray(stack, intToString(goToState));
      lexNext = false;
    } else if (action->action->type == ACCEPT) {
      return root;
    }
  }
  return NULL;
}

ParseTreeNode *popParseTreeNode(ParseTreeNode **stack) {
  for (int i = 1; i < 1024; i++)
    if (!stack[i] && stack[i - 1]) {
      ParseTreeNode *value = stack[i - 1];
      stack[i - 1] = NULL;
      return value;
    }
  return NULL;
}

void addParseTreeNodePtrToArray(ParseTreeNode **array, ParseTreeNode *value) {
  for (int i = 0; i < 1024; i++)
    if (!array[i]) {
      array[i] = value;
      break;
    }
}

char *getParseTreeAsString(ParseTreeNode *node) {
  char *result = calloc(100000, sizeof(char));
  if (!node) return result;
  ParseTreeNode **stack = calloc(1024, sizeof(char *));
  addParseTreeNodePtrToArray(stack, node);
  while (true) {
    node = popParseTreeNode(stack);
    if (!node) break;
    sprintf(result + strlen(result), "%s\n", node->value);
    for (int i = 0; i < 1024; i++)
      if (node->children[i])
        addParseTreeNodePtrToArray(stack, node->children[i]);
  }
  return result;
}

