#include "libcfu/cfustring.c"
#include "libcfu/cfuhash.c"

#include "linked_list.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct value {
    char *addr;
    int num;
};

int main() {
    cfuhash_table_t *hash = cfuhash_new(30);
	size_t i;

	cfuhash_set_flag(hash, CFUHASH_FROZEN_UNTIL_GROWS);

    for (i = 1; i <= 10; i++) {
        char key[16];
        //struct value value_struct;
        node_t *value = new_linked_list("10.0.0.7");
        push(value, "10.0.0.42");
        sprintf(key, "10.0.0.%zu", i);
        //value_struct.addr = "10.0.0.7";
        //value_struct.num = 5;

        //cfuhash_put(hash, key, &value_struct);
        cfuhash_put(hash, key, value);
        ////void *r = NULL;
        ////cfuhash_put_data(hash, (const void *)key, -1, &value_struct, sizeof value_struct, &r);
	}

	cfuhash_pretty_print(hash, stdout);

    //struct value *nn;
    //nn = (struct value *) cfuhash_get(hash, "10.0.0.2");
    //printf("Value for key '10.0.0.2'='%s'\n", (*nn).addr);
    printf("%s","Value for key '10.0.0.2'=");
    print_list(cfuhash_get(hash, "10.0.0.2"));
    puts("");

	printf("10.0.0.11 %s\n", cfuhash_exists(hash, "10.0.0.11") ? "exists" : "does NOT exist!!!");

	printf("%zu entries, %zu buckets used out of %zu\n", cfuhash_num_entries(hash), cfuhash_num_buckets_used(hash), cfuhash_num_buckets(hash));

    //nn = (struct value *)cfuhash_delete(hash, "10.0.0.2");
    printf("%s", "deleted value/key pair: (10.0.0.2, ");
    print_list(cfuhash_delete(hash, "10.0.0.2"));
    puts(")");

	cfuhash_pretty_print(hash, stdout);
	
	cfuhash_clear(hash);
    return 0;
}