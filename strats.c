#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GUSHERS 10
int num_gushers;
char names[MAX_GUSHERS];
int penalties[MAX_GUSHERS];
bool connects[MAX_GUSHERS * MAX_GUSHERS];

#define TREE_SIZE (1 << MAX_GUSHERS)
char cur_tree[TREE_SIZE];
bool eliminated[TREE_SIZE][MAX_GUSHERS];
bool path_visited[MAX_GUSHERS];



#define for_gusher(gusher) for (char gusher = 0; gusher < num_gushers; gusher++)

enum gen_res {
	FOUND_NEXT,
	IMPOSSIBLE,
};

bool is_connected(char x, char y) {
	return connects[x * num_gushers + y];
}

enum gen_res gen_tree(int idx) {
	int lo_idx = idx * 2;
	int hi_idx = lo_idx + 1;

	if (cur_tree[idx] == MAX_GUSHERS) {
		for_gusher(gusher) {
			if (!eliminated[idx][gusher])
				goto possible;
		}

		return IMPOSSIBLE;

possible:
		for_gusher(gusher) {
			if (path_visited[gusher])
				continue;

			cur_tree[idx] = gusher;
			path_visited[gusher] = true;
			for_gusher(n_gusher) {
				if (path_visited[n_gusher] || eliminated[idx][n_gusher]) {
					eliminated[lo_idx][n_gusher] = true;
					eliminated[hi_idx][n_gusher] = true;
				} else if (is_connected(gusher, n_gusher)) {
					eliminated[lo_idx][n_gusher] = true;
					eliminated[hi_idx][n_gusher] = false;
				} else {
					eliminated[lo_idx][n_gusher] = false;
					eliminated[hi_idx][n_gusher] = true;
				}
			}
			cur_tree[lo_idx] = cur_tree[hi_idx] = MAX_GUSHERS;

			gen_tree(lo_idx);
			gen_tree(hi_idx);

			path_visited[gusher] = false;
			return FOUND_NEXT;
		}

		assert(false);
	} else {
		path_visited[cur_tree[idx]] = true;

		if (gen_tree(lo_idx) == FOUND_NEXT)
			goto inner_found_next;
		if (gen_tree(hi_idx) == FOUND_NEXT)
			goto inner_found_next;

		path_visited[cur_tree[idx]] = false;

		for_gusher(gusher) {
			if (path_visited[gusher] || gusher <= cur_tree[idx])
				continue;

			cur_tree[idx] = gusher;
			path_visited[gusher] = true;
			for_gusher(n_gusher) {
				if (path_visited[n_gusher] || eliminated[idx][n_gusher]) {
					eliminated[lo_idx][n_gusher] = true;
					eliminated[hi_idx][n_gusher] = true;
				} else if (is_connected(gusher, n_gusher)) {
					eliminated[lo_idx][n_gusher] = true;
					eliminated[hi_idx][n_gusher] = false;
				} else {
					eliminated[lo_idx][n_gusher] = false;
					eliminated[hi_idx][n_gusher] = true;
				}
			}
			cur_tree[lo_idx] = cur_tree[hi_idx] = MAX_GUSHERS;

			gen_tree(lo_idx);
			gen_tree(hi_idx);

			path_visited[gusher] = false;
			return FOUND_NEXT;
		}

		return IMPOSSIBLE;

inner_found_next:
		path_visited[cur_tree[idx]] = false;
		return FOUND_NEXT;
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: strats [filename.bin]\n");
		exit(1);
	}
	FILE *f = fopen(argv[1], "r");
	if (!f) {
		perror(argv[1]);
		exit(1);
	}

	if (fread(&num_gushers, sizeof(num_gushers), 1, f) != 1) {
		fprintf(stderr, "Error: Format\n");
		exit(1);
	}
	if (num_gushers >= MAX_GUSHERS) {
		fprintf(stderr, "Error: Format\n");
		exit(1);
	}
	if (fread(&names, sizeof(*names), num_gushers, f) != num_gushers) {
		fprintf(stderr, "Error: Format\n");
		exit(1);
	}
	if (fread(&penalties, sizeof(*penalties), num_gushers, f) != num_gushers) {
		fprintf(stderr, "Error: Format\n");
		exit(1);
	}
	if (fread(&connects, sizeof(*connects), num_gushers * num_gushers, f) !=
	    num_gushers * num_gushers) {
		fprintf(stderr, "Error: Format\n");
		exit(1);
	}
	fclose(f);

	memset(cur_tree, MAX_GUSHERS, 1 << (num_gushers + 1));

	int cnt = 0;
	while (gen_tree(1) == FOUND_NEXT) cnt++;
	printf("%d\n", cnt);

	return 0;
}
