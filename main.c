#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "main.h"
#include "malloc_free.h"
#include "tests.h"

#pragma region Helpers

/* Take an integer and print it in constant size format [* prefix:    number *] */
void print_formatted(char *prefix, uint64_t number)
{
    size_t format_size = strlen("***********************");
    char num_string[20];
    sprintf(num_string, "%ld", number);
    int spaces = (format_size - 4) - strlen(num_string) - strlen(prefix);

    char formatted[50] = {0};

    formatted[0] = '*';
    formatted[1] = ' ';
    strcat(formatted, prefix);
    for (size_t i = 0; i < spaces; i++)
    {
        strcat(formatted, " ");
    }
    strcat(formatted, num_string);
    strcat(formatted, " *");

    printf("%s\n", formatted);
}

/* Walks through free list and prints out info */
void walk_free_list()
{
    printf("Walking through free list...\n");

    node *curr = free_list_head;
    int num_free_chunks = 0;

    printf("%p\n", curr);

    while (curr)
    {
        num_free_chunks++;
        printf("Free chunk at address %ld with size %ld and next %ld\n", (uint64_t)curr - offset, (uint64_t)curr->size, curr->next ? (uint64_t)curr->next - offset : 0);
        curr = curr->next;
    }
    printf("There %s %d free chunk%s\n", num_free_chunks == 1 ? "is" : "are", num_free_chunks, num_free_chunks == 1 ? "" : "s");
}

/* Walks through allocated chunks and prints out info */
void walk_allocated_chunks()
{
    printf("Walking through allocated chunks...\n");

    void *address = heap_pointer;
    node *last_free = free_list_head;

    int num_allocated_chunks = 0;

    while (address < heap_pointer + HEAP_SIZE)
    {
        // If it is free
        // check if it is in free list
        if (address == last_free)
        {
            node *chunk = (node *)address;

            // next free chunk
            last_free = last_free->next;
            // next chunk
            address += (chunk->size + sizeof(node));
        }
        // else it must be allocated
        else
        {
            header *chunk = (header *)address;
            // check magic number is right
            assert(chunk->magic == MAGIC_NUMBER);

            num_allocated_chunks++;

            // print out allocated chunk info
            printf("Allocated chunk at address %ld with size %ld and magic %d\n", (uint64_t)chunk - offset, chunk->size, chunk->magic);

            // next chunk
            address += (chunk->size + sizeof(header));
        }
    }
    printf("There %s %d allocated chunk%s\n", num_allocated_chunks == 1 ? "is" : "are", num_allocated_chunks, num_allocated_chunks == 1 ? "" : "s");
}

/* Walk through heap and print everything in an ascii diagram. Verifies all memory is accounted for.*/
void audit()
{
    printf("\n================\n");
    printf("==  AUDITING  ==\n");
    printf("================\n");

    printf("Heap start: %ld\n", (uint64_t)heap_pointer - offset);
    printf("Heap size: %ld\n", HEAP_SIZE);
    printf("Free list start: %ld\n\n", (uint64_t)free_list_head - offset);

    // WALK BY CHUNK
    // SIMULTANEOUSLY WALK FREE LIST WHEN FINDING A FREE CHUNK

    void *address = heap_pointer;
    node *last_free = free_list_head;

    int num_allocated_chunks = 0;
    int num_free_chunks = 0;

    while (address < heap_pointer + HEAP_SIZE)
    {
        // Check for alignment
        assert(((uint64_t)address - offset) % 8 == 0);

        // If it is free
        // check if it is in free list
        if (address == last_free)
        {
            num_free_chunks++;
            node *chunk = (node *)address;

            // print data
            printf("***********************\n");
            printf("*      FREE CHUNK     *\n");
            print_formatted("Address: ", (uint64_t)address - offset);
            printf("***********************\n");
            print_formatted("Size: ", (uint64_t)chunk->size);
            print_formatted("Next: ", chunk->next ? (uint64_t)chunk->next - offset : 0);
            printf("*                     *\n");
            printf("***********************\n");

            // next free chunk
            last_free = last_free->next;
            // next chunk
            address += (chunk->size + sizeof(node));
        }
        // else it must be allocated
        else
        {
            num_allocated_chunks++;
            header *chunk = (header *)address;
            // check magic number is right
            assert(chunk->magic == MAGIC_NUMBER);

            // print data
            printf("***********************\n");
            printf("*   ALLOCATED CHUNK   *\n");
            print_formatted("Address: ", (uint64_t)address - offset);
            printf("***********************\n");
            print_formatted("Size: ", chunk->size);
            print_formatted("Magic: ", chunk->magic);
            printf("*                     *\n");
            printf("***********************\n");

            // next chunk
            address += (chunk->size + sizeof(header));
        }
        // Make it more legible
        if (address < heap_pointer + HEAP_SIZE)
        {
            printf("        |    |        \n");
            printf("        |    |        \n");
        }
    }

    assert((uint64_t)address - offset == HEAP_SIZE);
    printf("Accounted for %ld of %ld bytes in heap\n", (uint64_t)address - offset, HEAP_SIZE);
    printf("There %s %d allocated chunk%s\n", num_allocated_chunks == 1 ? "is" : "are", num_allocated_chunks, num_allocated_chunks == 1 ? "" : "s");
    printf("There %s %d free chunk%s\n\n", num_free_chunks == 1 ? "is" : "are", num_free_chunks, num_free_chunks == 1 ? "" : "s");
}

#pragma endregion Helpers

#pragma region Shell

/* NOT USED. Walks through the heap till finding an allocated chunk at the specified index. Prints an error message if there is no such allocated chunk. */
void free_at_index(int index)
{
    // Return if index is less than 1
    if (index < 1)
    {
        printf("Index must be at least 1\n");
        return;
    }

    // WALK BY CHUNK
    // SIMULTANEOUSLY WALK FREE LIST WHEN FINDING A FREE CHUNK

    void *address = heap_pointer;
    node *last_free = free_list_head;

    int allocated_chunk_index = 0;

    while (address < heap_pointer + HEAP_SIZE)
    {
        // If it is free
        // check if it is in free list
        if (address == last_free)
        {
            node *chunk = (node *)address;

            // next free chunk
            last_free = last_free->next;
            // next chunk
            address += (chunk->size + sizeof(node));
        }
        // else it must be allocated
        else
        {
            header *chunk = (header *)address;
            // check magic number is right
            assert(chunk->magic == MAGIC_NUMBER);

            allocated_chunk_index++;
            // If this is the index to free then break out of loop
            if (allocated_chunk_index == index)
                break;

            // next chunk
            address += (chunk->size + sizeof(header));
        }
    }

    if (index > allocated_chunk_index)
    {
        printf("Index is greater than the number of allocated chunks!\n");
        printf("There are only %d allocated chunks\n", allocated_chunk_index);
    }
    else
    {
        printf("Freeing allocated chunk at index %d\n", allocated_chunk_index);
        printf("Freeing allocated chunk at address %ld\n", (uint64_t)address - offset);

        my_free(address + sizeof(header));
    }
}

/* Show a list of commands for the interactive shell. */
void show_commands()
{
    printf("\naudit - Audits the heap and displays it in diagram format\n");
    printf("walk free - Walks through the free list and prints out info\n");
    printf("walk allocated - Walks through the allocated chunks and prints out info\n");
    printf("malloc - Allocates a chunk of a user specified size\n");
    printf("free - Frees the allocated chunk at the address specified by the user\n");
    printf("test - Select a test to run\n");
    printf("reset - Clears the heap of allocated chunks\n");
    printf("help - Displays this list of commands\n");
    printf("quit - End the session\n\n");
}

/* Show the available tests. */
void show_tests()
{
    printf("             AVAILABLE TESTS             \n");
    printf("=========================================\n");
    printf("Please note: running a test will clear the heap\n\n");
    printf("all - run all tests in below order\n");
    printf("reuse - run free chunk reuse tests\n");
    printf("sorted - run sorted free list tests\n");
    printf("splitting - run splitting free chunks tests\n");
    printf("coalescing - run coalescing tests\n");
    printf("alternating - run alternating sequence tests\n");
    printf("fit - run worst fit tests\n");
    printf("return - run malloc bad value tests\n\n");
}

/* Run the selected test. */
void select_test(char *which)
{
    if (!strcmp(which, "all"))
    {
        test_all();
    }
    else if (!strcmp(which, "reuse"))
    {
        test_free_chunk_reuse();
    }
    else if (!strcmp(which, "sorted"))
    {
        test_sorted_free_list();
    }
    else if (!strcmp(which, "splitting"))
    {
        test_splitting_free_chunks();
    }
    else if (!strcmp(which, "coalescing"))
    {
        test_coalesce();
    }
    else if (!strcmp(which, "alternating"))
    {
        test_alternating_sequence();
    }
    else if (!strcmp(which, "fit"))
    {
        test_worst_fit();
    }
    else if (!strcmp(which, "return"))
    {
        test_malloc_bad_size();
    }
    else
    {
        printf("Unrecognized test selection. Type 'test' to see the list of available tests\n");
    }
}

/* Start the interactive shell */
void start_shell()
{
    char command[100];

    show_commands();

    while (strcmp(command, "quit"))
    {
        printf("> ");
        scanf("%s", command);

        // Which commands to execute
        if (!strcmp(command, "audit"))
        {
            audit();
        }
        else if (!strcmp(command, "walk"))
        {
            char which[20];
            scanf("%s", which);
            if (!strcmp(which, "free"))
            {
                walk_free_list();
            }
            else if (!strcmp(which, "allocated"))
            {
                walk_allocated_chunks();
            }
            else
            {
                printf("Invalid command for 'walk'. Use 'free' or 'allocated'\n");
            }
        }
        else if (!strcmp(command, "malloc"))
        {
            int size = 0;
            printf("Size of chunk to allocate: ");
            scanf("%d", &size);
            printf("You requested to allocate a chunk of size %d\n", size);
            my_malloc(size);
        }
        else if (!strcmp(command, "free"))
        {
            int address = -1;
            printf("Address of allocated chunk to free (address displayed in audit): ");
            scanf("%d", &address);
            printf("You requested to free allocated chunk at address %d\n", address);

            // Make sure it is valid
            if (address < 0 || address + sizeof(header) > HEAP_SIZE)
            {
                printf("That address is not valid\n");
                continue;
            }

            header *chunk = (header *)(address + offset);
            if (chunk->magic == MAGIC_NUMBER)
            {
                my_free(chunk + 1);
                printf("Freed chunk at address %d\n", address);
            }
            else
            {
                printf("No allocated chunk found at address %d\n", address);
                printf("Try using 'audit' or 'walk allocated' to see the addresses of allocated chunks\n");
            }
        }
        else if (!strcmp(command, "test"))
        {
            show_tests();
            char which[20];
            printf("Which test to run: ");
            scanf("%s", which);
            select_test(which);
        }
        else if (!strcmp(command, "reset"))
        {
            free_all_chunks();
        }
        else if (!strcmp(command, "help"))
        {
            show_commands();
        }
        else if (strcmp(command, "quit"))
        {
            printf("Unrecognized command. Type 'help' to see the list of commands.\n");
        }
    }
    printf("\nSession ended\n");
}

#pragma endregion Shell

int main(int argc, char const *argv[])
{
    init_heap();
    init_tests();

    if (argv[1])
    {
        test_all();
    }
    else
    {
        start_shell();
    }
}