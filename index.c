#include "index.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TrieKey FLAG_END = 128;

static TrieNode_t* trie_node_create(TrieKey key, bool end);
static TrieNode_t* trie_node_insert(TrieNode_t* head, TrieKey key, bool end);
static TrieNode_t* trie_find(const TrieNode_t* head, TrieKey key);

timeval utils_get_time()
{
    timeval t;
    gettimeofday(&t, NULL);
    return t;
}

double utils_get_time_duration_ms(timeval start)
{
    timeval end;
    gettimeofday(&end, NULL);

    double duration = (end.tv_sec - start.tv_sec) * 1000.0;
    duration += (end.tv_usec - start.tv_usec) / 1000.0;
    return duration;
}

size_t utils_trim_line(char* line, size_t size)
{
    if (size > 0 && line[size - 1] == '\n') {
        line[size - 1] = '\0';
        size--;
    }
    return size;
}

static bool node_is_end(const TrieNode_t* node)
{
    return (node->key & FLAG_END) > 0;
}

static void node_set_end(TrieNode_t* node, bool end)
{
    node->key |= FLAG_END;
}

static TrieKey node_get_key(const TrieNode_t* node)
{
    return node->key & ~FLAG_END;
}

static int32_t node_compare(const TrieNode_t* node, TrieKey key)
{
    TrieKey k = node_get_key(node);
    if (k == key)
        return 0;
    return key < k ? -1 : 1;
}

static BTNode_t* trie_childs_create_node(TrieKey key, bool end)
{
    BTNode_t* bst_node = (BTNode_t*) malloc(sizeof(BTNode_t));
    bst_node->left = NULL;
    bst_node->right = NULL;
    bst_node->data = trie_node_create(key, end);
    return bst_node;
}

static void trie_childs_destroy_node(BTNode_t* node)
{
    if (node) {
        if (node->data)
            free(node->data);
        free(node);
    }
}

static BTNode_t* trie_childs_insert(BTNode_t** head, TrieKey key, bool end)
{
    if (*head) {
        if (node_compare((*head)->data, key) < 0)
            return trie_childs_insert(&(*head)->left, key, end);
        else
            return trie_childs_insert(&(*head)->right, key, end);
    } else {
        *head = trie_childs_create_node(key, end);
    }
    return *head;
}

static BTNode_t* trie_childs_find(BTNode_t* head, TrieKey key)
{
    BTNode_t* node = head;
    while (node) {
        int32_t res = node_compare(node->data, key);
        if (res == 0)
            return node;
        else if (res < 0)
            node = node->left;
        else
            node = node->right;
    }
    return 0;
}

static void trie_childs_destroy(BTNode_t* head)
{
    if (!head)
        return;

    if (head->data->childs)
        trie_childs_destroy(head->data->childs);

    if (head->left)
        trie_childs_destroy(head->left);
    if (head->right)
        trie_childs_destroy(head->right);

    trie_childs_destroy_node(head->left);
    trie_childs_destroy_node(head->right);
}

static TrieNode_t* trie_node_create(TrieKey key, bool end)
{
    TrieNode_t* node = (TrieNode_t*) malloc(sizeof(TrieNode_t));
    node->key = key;
    node->childs = NULL;
    if (end)
        node_set_end(node, end);
    return node;
}

static TrieNode_t* trie_node_insert(TrieNode_t* head, TrieKey key, bool end)
{
    BTNode_t* node = trie_childs_insert(&head->childs, key, end);
    return node->data;
}

static TrieNode_t* trie_insert(Index_t* index, TrieNode_t* head, TrieKey key, bool end)
{
    index->symbols++;
    if (end)
        index->words++;

    TrieNode_t* node = trie_find(head, key);
    if (node) {
        if (end && !node_is_end(node))
            node_set_end(node, end);
        return node;
    }
    node = trie_node_insert(head, key, end);
    index->nodes++;
    return node;
}

static TrieNode_t* trie_find(const TrieNode_t* head, TrieKey key)
{
    BTNode_t* node = trie_childs_find(head->childs, key);
    return node ? node->data : 0;
}

Index_t* index_create()
{
    Index_t* index = (Index_t*) malloc(sizeof(Index_t));
    index->max_string_size = 0;
    index->symbols = 0;
    index->nodes = 0;
    index->words = 0;
    index->head = trie_node_create(0, false);
    return index;
}

Index_t* index_create_from_file(char* filename, bool dump)
{
    if (!filename)
        return NULL;

    FILE* fp = fopen(filename, "r");
    if (!fp)
        return NULL;

    const size_t dump_progress_lines = 100000;
    Index_t* index = index_create();
    timeval start = utils_get_time();

    char* line = NULL;
    size_t size = 0;
    size_t length = 0;
    size_t lines = 0;

    if (dump) {
        printf("Indexing of %s in progress...\n", filename);
    }

    while ((size = getline(&line, &length, fp)) != -1) {
        size = utils_trim_line(line, size);

        index_insert(index, line, size);

        if (dump) {
            lines++;
            if (lines % dump_progress_lines == 0) {
                printf("Indexing... %zu lines ~ %.4f (s). ", lines, (utils_get_time_duration_ms(start) / 1000));
                index_dump(index);
            }
        }
    }

    if (dump) {
        printf("Indexing done! ~ %.4f (s).\n", (utils_get_time_duration_ms(start) / 1000));
        index_dump(index);
    }

    if (line)
        free(line);
    fclose(fp);
    return index;
}

void index_destroy(Index_t* index)
{
    if (!index)
        return;

    TrieNode_t* head = index->head;
    if (head) {
        trie_childs_destroy(head->childs);
        trie_childs_destroy_node(head->childs);
    }

    free(head);
    free(index);
}

void index_insert(Index_t* index, const char* string, size_t size)
{
    if (!string)
        return;

    index->max_string_size = max(index->max_string_size, size);

    TrieNode_t* node = index->head;
    for (size_t i = 0; i < size; i++) {
        node = trie_insert(index, node, string[i], i == size - 1);
    }
}

bool index_find(Index_t* index, const char* string, size_t size)
{
    if (!string || size == 0)
        return false;

    bool result = false;
    if (size <= index->max_string_size) {
        TrieNode_t* node = index->head;
        for (size_t i = 0; i < size; ++i) {
            node = trie_find(node, string[i]);
            if (!node)
                break;
        }
        result = node && node_is_end(node);
    }
    return result;
}

void index_dump(Index_t* index)
{
    float ratio = index->symbols ? ((float) index->nodes / index->symbols * (sizeof(char))) : 0;
    float ratioSize = ratio * (sizeof(BTNode_t) + sizeof(TrieNode_t));
    printf("=> words=%zu, n=%zu, ratio=%.2f, ratioSize=%.2f, size=%.4f (mb)\n",
           index->words, index->symbols, ratio, ratioSize, ((float) index->symbols) / 1024 / 1024);
}