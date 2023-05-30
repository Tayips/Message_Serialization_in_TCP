#ifndef LINKEDLIST_H
#define LINKEDLIST_H

struct Llist {
    int client_id;
    struct Llist *next;
};

void create(int c_id);
void findandremove(int c_id);
char* print(int c);
int isValid(int c, int clientfd);

#endif /* LINKEDLIST_H */
