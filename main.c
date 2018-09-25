/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/
//By:Michaelpeng65535@gmail.com
#include <stdio.h>

typedef enum { false, true } bool;
int global_id;

typedef struct node {
	unsigned char mac[12];
	int id;
	bool is_controler;
	int depth;
	struct node *brothers;
	struct node *child;
	struct node *parents;//if we further need pick branchs
	struct node *prev;
} node;


node *create_node(node *parents, node *prev, unsigned char *mac, int depth);
node *add_sibling(node *parents, node *brother, unsigned char *mac, int depth);
node *add_child(node *parents, unsigned char *mac, int depth);
node* search(node *cur, unsigned char *mac);
void pick_up_branchs(node *root, unsigned char (*branch)[12], int n);
void remove_node(node *node);
node* remove_node_helper(node *target_node);
void free_tree(node *node);
void test_helper(node *root);
void test_helper_branchs(node *root);

int main()
{
    int i = 0;
    unsigned char my_mac[12] = "111111111111";
    node *root = create_node(NULL, NULL, my_mac, 1);
    root->is_controler = true;
    root->id = ++global_id;
    test_helper(root);
    //test_helper_branchs(root);
    return 0;
}

node *create_node(node *parents, node *prev, unsigned char *mac, int depth)
{
    node *new_node = malloc(sizeof(node));
    
    if (new_node) {
        memcpy(new_node->mac, mac, 12);
        new_node->parents = parents;
        new_node->brothers = NULL;
        new_node->child = NULL;
        new_node->prev = prev;
        new_node->is_controler = false;
        new_node->id = ++global_id;
        new_node->depth = depth;
        printf("created node at level %d with mac %c%c\n", new_node->depth, new_node->mac[0], new_node->mac[1]);
    } else
        printf ("oops malloc fail\n");
    return new_node;
}

void remove_node(node *target_node)
{
    node *parent = target_node->parents;
    node *prev = target_node->prev;
    node *new_node = NULL;
    if (target_node->parents == NULL)
        remove_node_helper(target_node); //Red wedding
    else if ((target_node->brothers == NULL) && (prev == NULL)) {
        parent->child = NULL;
        remove_node_helper(target_node);
    } else if ((target_node->brothers == NULL) && (prev != NULL)) {//extra words to easier understand
        prev->brothers = NULL;
        remove_node_helper(target_node);
    } else if ((target_node->brothers != NULL) && (prev == NULL)) {
        new_node = remove_node_helper(target_node);
        parent->child = new_node;
        new_node->prev = NULL;
    } else {
        new_node = remove_node_helper(target_node);
        prev->brothers = new_node;
        new_node->prev = prev;
    }
}

node* remove_node_helper(node *target_node)
{
    node *new_node = NULL;
    if (target_node) {
       while(target_node->child) {
           target_node->child = remove_node_helper(target_node->child);
       }
       new_node = target_node->brothers;
       printf("removed node %c%c\n", target_node->mac[0], target_node->mac[1]);
       free(target_node);
    }
    return new_node; 
}

void free_tree(node *target_node){
    if(target_node){
        if (target_node->brothers)
            free_tree(target_node->brothers);
        if (target_node->child)
            free_tree(target_node->child);
        printf("removed node %c%c\n", target_node->mac[0], target_node->mac[1]);
        free(target_node);
    }
}
node *add_sibling(node *parents, node *brother, unsigned char *mac, int depth)
{

    while (brother->brothers) {
        if (!memcmp(brother->brothers->mac, mac, 12))//skip node with same mac 
            return NULL;
        brother = brother->brothers;
    }
    brother->brothers = create_node(parents, brother, mac, depth);
    return brother->brothers;
}

node *add_child(node *parents, unsigned char *mac, int depth)
{
    if (parents == NULL)
        return NULL;
    node *res;
    if (parents->child) {
        if (!memcmp(parents->child->mac, mac, 12))//skip node with same mac 
            return NULL;
        res = add_sibling(parents, parents->child, mac, parents->child->depth);
    }
    else {
        parents->child = create_node(parents, NULL, mac, ++depth);
        res = parents->child;
    }
    return res;
}

node* search(node *cur, unsigned char *mac) //dfs
{
    node* res;
    if(cur == NULL)
        return NULL;
    
    if (!memcmp(mac, cur->mac,12)) {
        printf("we find it! -----> the dep %d, mac %c%c\n", cur->depth, cur->mac[0], cur->mac[1]);
        return cur;
    }
    
   // printf("the dep %d, mac %c%c\n", cur->depth, cur->mac[0], cur->mac[1]);
    
    if (cur->child != NULL)
        cur = cur->child;
    else 
        return NULL;
        
    while (cur != NULL) {
        if((res = search(cur, mac)) != NULL)
            return res;
        cur = cur->brothers;
    }
    return NULL;
}

void pick_up_branchs(node *root, unsigned char (*branch)[12], int n) {
    node *found_node, *center_node;
    int found = -1;
    int i = n;
    
    if ((found_node = search(root, branch[0])) != NULL) {
        while (i>0) {
            if (!memcmp(found_node->parents, branch[i-1], 12)) {//we compare the direction with what we have in hands 
                found = i;
                break;
            }
            i--;
        }
        
        if(found != -1) {
            i = n;
            while (i>0) {
                if (i != found && memcmp(found_node->mac, branch[i-1], 12))//skip known piece
                    add_child(found_node, branch[i-1], found_node->depth);
                i--;
            }
        } else 
            printf("invaild branch (condition 1)!!\n");//the branch is broken
    } else {
        while (i>0) {
            if (i != 1 && (found_node = search(root, branch[i-1])) != NULL) {
                found = i;
                break;
            }
            i--;
        }
        if (found != -1) { //one node(not center) overlaping
            i = n;
            center_node = add_child(found_node, branch[0], found_node->depth);
            while (i>0) {
                if (i != found && i != 1)//skip known piece
                    add_child(center_node, branch[i-1], center_node->depth);
                i--;
            }
        } else 
            printf("invaild branch (condition 2)!!\n");//the branch may ok but we cannot process now
    }
}

void test_helper(node *root) {
    
/*
I am not a good painter, but the tree is kind of like this:
          1
      2   3   4
   5      7      8
6    9
*/
    node *remove_target = NULL, *new_child = NULL;
    unsigned char my_mac[12] = "111111111111";
    unsigned char my_mac2[12] = "222222222222";
    unsigned char my_mac3[12] = "333333333333";
    unsigned char my_mac4[12] = "444444444444";
    unsigned char my_mac5[12] = "555555555555";
    unsigned char my_mac6[12] = "666666666666";
    unsigned char my_mac7[12] = "777777777777";
    unsigned char my_mac8[12] = "888888888888";
    unsigned char my_mac9[12] = "999999999999";
    add_child(root, my_mac2, root->depth);
    add_child(root, my_mac3, root->depth);
    add_child(root, my_mac4, root->depth);
    add_child(root->child, my_mac5, root->child->depth);
    add_child(root->child->child, my_mac6, root->child->child->depth);
    add_child(root->child->child, my_mac9, root->child->child->depth);
    add_child(root->child->brothers, my_mac7, root->child->brothers->depth);
    add_child(root->child->brothers->brothers, my_mac8, root->child->brothers->brothers->depth);
    search(root, my_mac8);
    //lets remove 222222222222
    remove_target = search(root, my_mac2);
    remove_node(remove_target);
}

void test_helper_branchs(node *root) {
    //we need some test data, first one in the center of the piece
    unsigned char a_piece_of_branch[3][12] = {
        "222222222222",
        "111111111111",
        "555555555555"
    };
    
    unsigned char b_piece_of_branch[4][12] = {
        "555555555555",
        "222222222222",
        "666666666666",
        "999999999999"
    };
    
    unsigned char c_piece_of_branch[3][12] = {
        "333333333333",
        "111111111111",
        "777777777777"
    };
    
    unsigned char d_piece_of_branch[2][12] = {
        "777777777777",
        "333333333333"
    };
    
    unsigned char e_piece_of_branch[3][12] = {
        "444444444444",
        "888888888888",
        "111111111111"
    };

    printf("************************************************\n");
    pick_up_branchs(root, a_piece_of_branch, sizeof(a_piece_of_branch)/sizeof(a_piece_of_branch[0]));
    printf("************************************************\n");
    pick_up_branchs(root, b_piece_of_branch, sizeof(b_piece_of_branch)/sizeof(b_piece_of_branch[0]));
    printf("************************************************\n");
    pick_up_branchs(root, c_piece_of_branch, sizeof(c_piece_of_branch)/sizeof(c_piece_of_branch[0]));
    printf("************************************************\n");
    pick_up_branchs(root, d_piece_of_branch, sizeof(d_piece_of_branch)/sizeof(d_piece_of_branch[0]));
    printf("************************************************\n");
    pick_up_branchs(root, e_piece_of_branch, sizeof(e_piece_of_branch)/sizeof(e_piece_of_branch[0]));
    printf("************************************************\n");
    //Let's nuclear the tree
    free_tree(root);
}