#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GUSHERS 10
static int num_gushers;
static char names[MAX_GUSHERS];
static int penalties[MAX_GUSHERS];
static bool connects[MAX_GUSHERS * MAX_GUSHERS];

#define TREE_SIZE (1 << MAX_GUSHERS)
static char cur_tree[TREE_SIZE];
static bool eliminated[TREE_SIZE][MAX_GUSHERS];
static char num_possible[TREE_SIZE];
static bool path_visited[MAX_GUSHERS];

static int current_cost;
static int num_exits;
static float avg_penalty;
static float current_var;

#define for_gusher(gusher) for (char gusher = 0; gusher < num_gushers; gusher++)

enum gen_res {
	FOUND_NEXT,
	IMPOSSIBLE,
};

static bool is_connected(char x, char y) {
	return connects[x * num_gushers + y];
}

static enum gen_res gen_tree(int idx) {
	int lo_idx = idx * 2;
	int hi_idx = lo_idx + 1;

	if (cur_tree[idx] == MAX_GUSHERS) {
		char possible_gusher;

		num_possible[idx] = 0;
		for_gusher(gusher) {
			if (!eliminated[idx][gusher]) {
				num_possible[idx]++;
				possible_gusher = gusher;
			}
		}

		if (!num_possible[idx])
			return IMPOSSIBLE;

		if (num_possible[idx] == 1) {
			cur_tree[idx] = possible_gusher;
			cur_tree[lo_idx] = cur_tree[hi_idx] = MAX_GUSHERS;
			return FOUND_NEXT;
		}

		for_gusher(gusher) {
			if (path_visited[gusher])
				continue;

			if (num_possible[idx] == 2 && eliminated[idx][gusher])
				continue;

			if (eliminated[idx][gusher]) {
				bool adj = false;
				for_gusher(n_gusher)
					if (!eliminated[idx][n_gusher] &&
					    is_connected(n_gusher, gusher))
						adj = true;

				if (!adj)
					continue;
			}


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
		if (num_possible[idx] == 1)
			return IMPOSSIBLE;

		path_visited[cur_tree[idx]] = true;

		if (gen_tree(lo_idx) == FOUND_NEXT)
			goto inner_found_next;
		if (gen_tree(hi_idx) == FOUND_NEXT) {
			cur_tree[lo_idx] = MAX_GUSHERS;
			gen_tree(lo_idx);
			goto inner_found_next;
		}

		path_visited[cur_tree[idx]] = false;

		for_gusher(gusher) {
			if (path_visited[gusher] || gusher <= cur_tree[idx])
				continue;

			if (num_possible[idx] == 2 && eliminated[idx][gusher])
				continue;

			if (eliminated[idx][gusher]) {
				bool adj = false;
				for_gusher(n_gusher)
					if (!eliminated[idx][n_gusher] &&
					    is_connected(n_gusher, gusher))
						adj = true;

				if (!adj)
					continue;
			}

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

static void compute_cost(int idx, int penalty) {
	int lo_idx = idx * 2;
	int hi_idx = lo_idx + 1;

	if (cur_tree[idx] == MAX_GUSHERS)
		return;

	if (!eliminated[idx][cur_tree[idx]]) {
		current_cost += penalty;
		num_exits++;
	}

	penalty += penalties[cur_tree[idx]];
	compute_cost(lo_idx, penalty);
	compute_cost(hi_idx, penalty);
}

static void compute_var(int idx, int penalty) {
	int lo_idx = idx * 2;
	int hi_idx = lo_idx + 1;

	if (cur_tree[idx] == MAX_GUSHERS)
		return;

	if (!eliminated[idx][cur_tree[idx]])
		current_var += (penalty - avg_penalty) * (penalty - avg_penalty);

	penalty += penalties[cur_tree[idx]];
	compute_var(lo_idx, penalty);
	compute_var(hi_idx, penalty);
}

static void print_tree(int idx) {
	int lo_idx = idx * 2;
	int hi_idx = lo_idx + 1;

	if (cur_tree[idx] == MAX_GUSHERS)
		return;

	printf("%c", names[cur_tree[idx]]);

	if (eliminated[idx][cur_tree[idx]])
		printf("*");

	if (cur_tree[lo_idx] == MAX_GUSHERS && cur_tree[hi_idx] == MAX_GUSHERS)
		return;

	printf("(");

	print_tree(hi_idx);

	printf(",");

	print_tree(lo_idx);

	printf(")");
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
	while (gen_tree(1) == FOUND_NEXT) {
		cnt++;
		printf("%d: ", cnt);
		print_tree(1);

		current_cost = current_var = num_exits = 0;
		compute_cost(1, 0);

		assert(num_exits == num_gushers);
		avg_penalty = (float)current_cost / num_gushers;
		compute_var(1, 0);

		printf(": %d %.02f\n", current_cost, sqrt(current_var / num_gushers));
	}

	return 0;
}
