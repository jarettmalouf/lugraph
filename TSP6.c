#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "location.h"
#include "lugraph.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include <stdlib.h>


typedef struct _location_in_use
{
	char name[3];
	location loc;
	bool visited;
} location_in_use;


typedef struct _pair
{
	int index_city1;
	int index_city2;
	double dist;
} pair;

typedef int make_iso_compilers_happy;


void print_location_in_use(location_in_use location)
{
	printf("%s ", location.name);
	printf("%lf ", location.loc.lat);
	printf("%lf ", location.loc.lon);
	if (location.visited)
	{
		printf("T");
	}
	else
	{
		printf("F");
	}
	putchar('\n');
}

// comparing two location_in_use structs for equality
bool cmp_loc_in_use(location_in_use l1, location_in_use l2)
{
	if (strcmp(l1.name, l2.name) == 0 && l1.loc.lat == l2.loc.lat
		&& l1.loc.lon == l2.loc.lon && l1.visited == l2.visited)
	{	
		return true;
	}
	return false;
}

/*
Given a list of pointers to locations in use,
char* name, _location loc, bool visited
e.g. SEA, 47.450000 -122.311667, false
this function will produce an ordered list of locations in use
based on the "nearest" algorithm
*/
void produce_nearest(int locations_length, location_in_use locations[])
{
	double total_dist = 0;

	location_in_use n_arr[locations_length + 1];

	for (int i = 0; i < locations_length; i++)
	{
		locations[i].visited = false;
		n_arr[i].visited = false;
	}
	n_arr[locations_length].visited = false;

	n_arr[0] = locations[0];
	locations[0].visited = true;

	int counter = 1; // how many locations we've locked into n_arr

	int i = 0; // the location we're gonna start working on

	while (locations_length > counter)
	{
		int j = 0; // the point we're gonna compare with i
		while (locations[j].visited)
		{
			j++;
		}

		// we have j –– the lowest location in locations[]
		// we're gonna set this j to closest, by default

		location_in_use closest = locations[j];
		double min_dist = location_distance(&locations[i].loc, &locations[j].loc);

		// printf("Finding distance between %s %s : %lf\n", 
		// 	locations[i].name, locations[j].name,
		// 	min_dist);
		int min_place = j;

		// checking if other points are closer
		for (int k = j+1; k < locations_length; k++)	
		{
			double c;
			// if (!locations[k].visited)
			// {
			// printf("Finding distance between %s %s : %lf\n", 
			// 		locations[i].name, locations[k].name,
			// 		location_distance(&locations[i].loc, &locations[k].loc));
			// }	

			if (!locations[k].visited && ((c = location_distance(&locations[i].loc, &locations[k].loc)) < min_dist))
			{
				min_dist = c;
				min_place = k;
				closest = locations[k];
				
			}
		}

		// at this point, closest will be the location_in_use closest to i
		// and min_dist will be the distance between i and closest (either j or one of k's)

		n_arr[counter] = closest;
		// printf("Closest: %s, dist: %lf\n", locations[min_place].name, location_distance(&locations[i].loc, &locations[min_place].loc));
		locations[min_place].visited = true;
		counter++;
		i = min_place;
		total_dist += min_dist;

		// re-include the head as the tail
		if (locations_length == counter)
		{
			n_arr[counter] = locations[0];
			total_dist += location_distance(&locations[i].loc, &locations[0].loc);
			i = 0;
		}
	}

	// print n_arr

	printf("-nearest        :%10.2lf", total_dist);
	for (int x = 0; x <= locations_length; x++)
	{
		printf(" %s", n_arr[x].name);
	}
	putchar('\n');

}

/*
Given a list of pointers to locations in use,
char* name, _location loc, bool visited
e.g. SEA, 47.450000 -122.311667, false
this function will produce an ordered list of locations in use
based on the "insert nearest" algorithm
*/
void produce_ins_nearest(int locations_length, location_in_use locations[])
{
	// ni_arr = nearest insert array
	location_in_use ni_arr[locations_length + 1];

	for (int i = 0; i < locations_length; i++)
	{
		locations[i].visited = false;
		ni_arr[i].visited = false;
	}
	ni_arr[locations_length].visited = false;

	// Step 1: get initial pair A B

	double min_dist = DBL_MAX;
	int A, B;
	double total_dist = 0;

	for (int i = 0; i < locations_length; i++)
	{
		for (int j = i+1; j < locations_length; j++)
		{
			double c;
			if ((c = location_distance(&locations[i].loc, &locations[j].loc)) < min_dist)
			{
				min_dist = c;
				A = i;       // 	one constituent of first pair
				B = j;       //     other constituent of first pair
			}
		}
	}

	ni_arr[0] = locations[A];
	ni_arr[1] = locations[B];
	ni_arr[2] = locations[A];

	locations[A].visited = true;
	locations[B].visited = true;

	total_dist += (2.0 * min_dist);

	// now we have marked our first, closest pair AB
	// A, B, P_1, P_2, X, etc. are ints that represent the location's place in locations
	// SEA WYR TIO LEW ENO
	// A = 1 if A were to represent WYR
	// end of Step 1


	for (int i = 2; i < locations_length; i++)
	{
		// Step 2: permute A and B with rest of locations
		// in order to find the shortest segment besides AB
		
		double min_seg_dist = DBL_MAX;
		int X;

		for (int j = 0; j < locations_length; j++)
		{
			if (locations[j].visited)
			{
				for (int k = 0; k < locations_length; k++)
				{
					if (!locations[k].visited)
					{
						double c;
						if ((c = location_distance(&locations[j].loc, &locations[k].loc)) < min_seg_dist)
						{
							min_seg_dist = c;
							X = k;         //   the dot closest to a point in the tour
						}
					}
				}
			}
		}

		// now we've found the closest point X off-tour

		locations[X].visited = true;

		// Step 3: find P_1, P_2
		// P_2 : the  points in the tour that will touch the incoming dot X
		// X will eventually be inserted between P_1 and P_2
		// in the first iteration, it will only have one choice to test: 
		// the unchosen, further, non-P1 member of {A, B}

		// Rule: for a set {A, B, C, ...} of visited, non-P_1, non-X points,
		// and given P_1 and X, each candidate for P_2 represented by P_2*,
		// P_2 can be found with the following formula:
		// P_2 = min{dist(P_2*, X) - dist(P_2*, P_1)}

		double min_dist_added = DBL_MAX;
		int P2_pos_ni_arr;

		for (int a = 0; a < i; a++)
		{
			double c = (location_distance(&ni_arr[a].loc, &locations[X].loc)
					     +  location_distance(&ni_arr[a+1].loc, &locations[X].loc)
				         -  location_distance(&ni_arr[a].loc, &ni_arr[a+1].loc));
			if (c < min_dist_added)
			{
				min_dist_added = c;					
				P2_pos_ni_arr = a+1;
			}	
		}

		// P_1 and P_2 are on-tour sandwich buns for X
		// printf("%s and %s are on-tour sandwich buns for %s\n", ni_arr[P1_pos_ni_arr].name, 
		// ni_arr[P2_pos_ni_arr].name, locations[X].name);
		
		total_dist += min_dist_added;

		// unnecessary to declare locations[P_2].visited = true; 
		// because P_2 is already a point

		// Step 4: insert X (into ni_arr)
		// we now have both P_1 and P_2 to sandwich our X in

		int X_pos = P2_pos_ni_arr;

		// making a space for X between P_1 and P_2
		for (int m = i+1; m > X_pos; m--)
		{
			ni_arr[m] = ni_arr[m-1];
		}

		ni_arr[X_pos] = locations[X];
	}

	// formatting

	// Step 1: shifting index to first point
	int index = 0;
	while (strcmp(ni_arr[index].name, locations[0].name) != 0)
	{
		index++;
	}
	
	// Step 2: reversing if necessary
	// necessary to reverse if the second city in locations
	// does not appear before the penultimate city
	// figures out whether second or penultimate comes first in locations
	int sec, pen;
	location_in_use second;
	location_in_use penultimate;

	if (index == 0)
	{
		second = ni_arr[1];
		penultimate = ni_arr[locations_length - 1];
	}
	else 
	{
		second = ni_arr[index+1];
		penultimate = ni_arr[index-1];
	}

	for (int a = 0; a < locations_length; a++)
	{
		if (strcmp(second.name, locations[a].name) == 0)
		{
			sec = a;
		}
		if (strcmp(penultimate.name, locations[a].name) == 0)
		{
			pen = a;
		}
	}

	// if second comes after penultimate (wrong), reverse ni_arr
	if (sec > pen)
	{
		// printf("reverse about to happen");
		location_in_use temp;
		for (int i = 1; i <= locations_length / 2; i++)
		{
			temp = ni_arr[i];
			ni_arr[i] = ni_arr[locations_length - i];
			ni_arr[locations_length - i] = temp;
		}
		index = locations_length - index;
	}

	// print ni_arr 

	printf("-insert nearest :%10.2lf", total_dist);
	if (index == 0)
	{
		for (int w = 0; w <= locations_length; w++)
		{
			printf(" %s", ni_arr[w].name);
		}
	}
	else
	{
		for (int y = index; y < locations_length; y++)
		{
			printf(" %s", ni_arr[y].name);
		}
		for (int z = 0; z <= index; z++)
		{
			printf(" %s", ni_arr[z].name);
		}
	}
	putchar('\n');

}

/*
Given a list of pointers to locations in use,
char* name, _location loc, bool visited
e.g. SEA, 47.450000 -122.311667, false
this function will produce an ordered list of locations in use
based on the "insert farthest" algorithm
*/
void produce_ins_farthest(int locations_length, location_in_use locations[])
{
	// ni_arr = nearest insert array
	location_in_use nf_arr[locations_length + 1];
	
	for (int i = 0; i < locations_length; i++)
	{
		locations[i].visited = false;
		nf_arr[i].visited = false;
	}
	nf_arr[locations_length].visited = false;

	// Step 1: get initial farthest pair A B

	double max_dist;
	int A, B;
	double total_dist = 0;

	for (int i = 0; i < locations_length; i++)
	{
		for (int j = i+1; j < locations_length; j++)
		{
			double c;
			if ((c = location_distance(&locations[i].loc, &locations[j].loc)) > max_dist)
			{
				max_dist = c;
				A = i;       // 	one constituent of first pair
				B = j;       //     other constituent of first pair
			}
		}
	}

	locations[A].visited = true;
	locations[B].visited = true;

	nf_arr[0] = locations[A];
	nf_arr[1] = locations[B];
	nf_arr[2] = locations[A];

	total_dist += (2 * max_dist);

	// now we have marked our first, closest pair AB
	// A, B, P_1, P_2, X, etc. are ints that represent the location's place in locations
	// SEA WYR TIO LEW ENO
	// A = 1 if A were to represent WYR
	// end of Step 1


	for (int i = 2; i < locations_length; i++)
	{
		// Step 2: permute A and B with rest of locations
		// in order to find the longest segment besides AB
		
		double max_seg_dist = DBL_MIN;
		int X;

		for (int j = 0; j < locations_length; j++)
		{
			if (locations[j].visited)
			{
				for (int k = 0; k < locations_length; k++)
				{
					if (!locations[k].visited)
					{
						double c;
						if ((c = location_distance(&locations[j].loc, &locations[k].loc)) > max_seg_dist)
						{
							max_seg_dist = c;
							X = k;         //   the dot farthest from a point in the tour
						}
					}
				}
			}
		}


		// it is unnecessary to set locations[P_1].visited to true
		// because P_1 is already a point on the tour, either {A, B}
		locations[X].visited = true;

		// P_1–X is the shortest possible segment
		// that includes a point in tour and out of tour

		// Step 3: find P_2
		// P_2 : the second point in the tour that will touch the incoming dot X
		// X will eventually be inserted between P_1 and P_2
		// in the first iteration, it will only have one choice to test: 
		// the unchosen, further, non-P1 member of {A, B}

		// Rule: for a set {A, B, C, ...} of visited, non-P_1, non-X points,
		// and given P_1 and X, each candidate for P_2 represented by P_2*,
		// P_2 can be found with the following formula:
		// P_2 = min{dist(P_2*, X) - dist(P_2*, P_1)}
		double min_dist_added = DBL_MAX;
		int P2_pos_nf_arr;
		for (int a = 0; a < i; a++)
		{
			double c = (location_distance(&nf_arr[a].loc, &locations[X].loc)
					     +  location_distance(&nf_arr[a+1].loc, &locations[X].loc)
				         -  location_distance(&nf_arr[a].loc, &nf_arr[a+1].loc));
			if (c < min_dist_added)
			{
				min_dist_added = c;					
				P2_pos_nf_arr = a+1;
			}	
		}

		total_dist += min_dist_added;


		// unnecessary to declare locations[P_2].visited = true; 
		// because P_2 is already a point

		// Step 4: insert P_2 (into nf_arr)
		// we now have both P_1 and P_2 to sandwich our X in

		int X_pos = P2_pos_nf_arr;

		// making a space for X between P_1 and P_2
		for (int m = i+1; m > X_pos; m--)
		{
			nf_arr[m] = nf_arr[m-1];
		}

		nf_arr[X_pos] = locations[X];
	}

	// formatting

	// Step 1: shifting
	// figures out whether second or penultimate comes first in locations

	// Step 1: shifting index to first point
	int index = 0;
	while (strcmp(nf_arr[index].name, locations[0].name) != 0)
	{
		index++;
	}
	
	// Step 2: reversing if necessary
	// necessary to reverse if the second city in locations
	// does not appear before the penultimate city
	// figures out whether second or penultimate comes first in locations
	int sec, pen;
	location_in_use second;
	location_in_use penultimate;

	if (index == 0)
	{
		second = nf_arr[1];
		penultimate = nf_arr[locations_length - 1];
	}
	else 
	{
		second = nf_arr[index+1];
		penultimate = nf_arr[index-1];
	}

	for (int a = 0; a < locations_length; a++)
	{
		if (strcmp(second.name, locations[a].name) == 0)
		{
			sec = a;
		}
		if (strcmp(penultimate.name, locations[a].name) == 0)
		{
			pen = a;
		}
	}

	// if second comes after penultimate (wrong), reverse ni_arr
	if (sec > pen)
	{
		// printf("reverse about to happen");
		location_in_use temp;
		for (int i = 1; i <= locations_length / 2; i++)
		{
			temp = nf_arr[i];
			nf_arr[i] = nf_arr[locations_length - i];
			nf_arr[locations_length - i] = temp;
		}
		index = locations_length - index;
	}

	// print ni_arr 

	printf("-insert farthest:%10.2lf", total_dist);
	if (index == 0)
	{
		for (int w = 0; w <= locations_length; w++)
		{
			printf(" %s", nf_arr[w].name);
		}
	}
	else
	{
		for (int y = index; y < locations_length; y++)
		{
			printf(" %s", nf_arr[y].name);
		}
		for (int z = 0; z <= index; z++)
		{
			printf(" %s", nf_arr[z].name);
		}
	}
	putchar('\n');

}



/* merge sorted arrays a1 and a2, putting result in out */
void
merge(int n1, const pair a1[], int n2, const pair a2[], pair out[])
{
    int i1;
    int i2;
    int iout;

    i1 = i2 = iout = 0;

    while(i1 < n1 || i2 < n2) {
        if(i2 >= n2 || ((i1 < n1) && (a1[i1].dist < a2[i2].dist))) {
            /* a1[i1] exists and is smaller */
            out[iout++] = a1[i1++];
        }  else {
            /* a1[i1] doesn't exist, or is bigger than a2[i2] */
            out[iout++] = a2[i2++];
        }
    }
}

/* sort a, putting result in out */
/* we call this mergeSort to avoid conflict with mergesort in libc */
void
mergeSort(int n, const pair a[], pair out[])
{
    pair *a1;
    pair *a2;

    if(n < 2) {
        /* 0 or 1 elements is already sorted */
        memcpy(out, a, sizeof(pair) * n);
    } else {
        /* sort into temp arrays */
        a1 = malloc(sizeof(pair) * (n/2));
        a2 = malloc(sizeof(pair) * (n - n/2));

        mergeSort(n/2, a, a1);
        mergeSort(n - n/2, a + n/2, a2);

        /* merge results */
        merge(n/2, a1, n - n/2, a2, out);

        /* free the temp arrays */
        free(a1);
        free(a2);
    }
}

void produce_greedy(int locations_length, location_in_use *locations)
{
	int pairs_length = (locations_length * (locations_length - 1)) / 2;
	pair *pairs = malloc(sizeof(pair) * pairs_length);
	if (pairs == NULL)
	{
		exit(1);
	}

	int count = 0;
	// int v = 1;

	for (int i = 0; i < locations_length - 1; i++)
	{
		for (int j = i + 1; j < locations_length; j++)
		{
			pairs[count].index_city1 = i;
			pairs[count].index_city2 = j;
			pairs[count].dist = location_distance(&locations[i].loc, &locations[j].loc);
			// printf("Dist: %lf\n", pairs[count].dist);
			count++;
		}
	}
	// printf("%d\n", v);
	// v++;

	// printf("PAIRS\n");
	// for (int i = 0; i < pairs_length; i++)
	// {
	// 	printf("Pair %d: %s %s %lf\n", i, locations[pairs[i].index_city1].name,
	// 		locations[pairs[i].index_city2].name, pairs[i].dist);
	// }

	pair *sorted_pairs = malloc(sizeof(pair) * pairs_length);
	mergeSort(pairs_length, pairs, sorted_pairs);

	// printf("SORTED PAIRS\n");
	// for (int i = 0; i < pairs_length; i++)
	// {
	// 	printf("Sorted Pair %d: %s %s %lf\n", i, locations[sorted_pairs[i].index_city1].name,
	// 		locations[sorted_pairs[i].index_city2].name, sorted_pairs[i].dist);
	// }

	// printf("%d\n", v);
	// v++;

	lugraph *tour = lugraph_create(locations_length);
	if (tour == NULL)
	{
		free(pairs);
		free(sorted_pairs);
		exit(1);
	}

	for (int i = 0; i < pairs_length; i++)
	{
		if (!lugraph_connected(tour, sorted_pairs[i].index_city1, sorted_pairs[i].index_city2) &&
			lugraph_degree(tour, sorted_pairs[i].index_city1) < 2 &&
			lugraph_degree(tour, sorted_pairs[i].index_city2) < 2)
		{
			lugraph_add_edge(tour, sorted_pairs[i].index_city1, sorted_pairs[i].index_city2);
		}
	}

	// printf("%d\n", v);
	// v++;

	int *extremes = malloc(sizeof(int) * 2);
	if (extremes == NULL)
	{
		free(pairs);
		free(sorted_pairs);
		free(tour);
		exit(1);
	}
	int c = 0;

	for (int i = 0; i < locations_length; i++)
	{
		if (lugraph_degree(tour, i) == 1)
		{
			extremes[c] = i;
			c++;
		}
	}

	// printf("%d\n", v);
	// v++;

	int len;
	lug_search *search = lugraph_bfs(tour, extremes[0]);
	if (search == NULL)
	{
		free(pairs);
		free(sorted_pairs);
		exit(1);
	}

	int *path = lug_search_path(search, extremes[1], &len);
	if (path == NULL)
	{
		free(pairs);
		free(sorted_pairs);
		free(search);
		exit(1);
	}


	// for (int i = 0; i < locations_length; i++)
	// {
	// 	printf("Path[%d] = %d\t", i, path[i]);
	// 	printf(" %s", locations[path[i]].name);
	// 	printf("\n");
	// }

	// formatting

	// Step 1: shifting
	// figures out whether second or penultimate comes first in locations

	// Step 1: shifting index to first point
	int index = 0;
	while (path[index] != 0)
	{
		index++;
	}

	// index index of starting point
	
	// Step 2: reversing if necessary
	// necessary to reverse if the second city in locations
	// does not appear before the penultimate city
	// figures out whether second or penultimate comes first in locations
	int second, penultimate;

	if (index == 0)
	{
		second = path[1];
		penultimate = path[locations_length - 1];
	}
	else 
	{
		second = path[index+1];
		penultimate = path[index-1];
	}

	// printf("Second = %d, Penultimate = %d\n", second, penultimate);

	// if second comes after penultimate (wrong), reverse ni_arr
	if (second > penultimate)
	{
		for (int i = 0; i < locations_length / 2; i++)
		{
			int temp = path[i];
			path[i] = path[locations_length - 1 - i];
			path[locations_length - 1 - i] = temp;
		}
		index = locations_length - index - 1;
	}

	// for (int i = 0; i < locations_length; i++)
	// {
	// 	printf("Path[%d] = %d\t", i, path[i]);
	// 	printf(" %s", locations[path[i]].name);
	// 		printf("\n");
	// }

	double total_dist = 0;

	if (index == 0)
	{
		for (int w = 0; w < locations_length; w++)
		{
			total_dist += location_distance(&locations[path[w % locations_length]].loc, &locations[path[(w+1) % locations_length]].loc);
		}
	}
	else
	{
		for (int y = index; y < locations_length + index; y++)
		{
			total_dist += location_distance(&locations[path[y % locations_length]].loc, &locations[path[(y+1) % locations_length]].loc);
		}
	}

	printf("-greedy         :%10.2lf", total_dist);
	if (index == 0)
	{
		for (int w = 0; w <= locations_length; w++)
		{
			printf(" %s", locations[path[w % locations_length]].name);
		}
	}
	else
	{
		for (int y = index; y <= locations_length + index; y++)
		{
			printf(" %s", locations[path[y % locations_length]].name);
		}
	}
	putchar('\n');

	free(pairs);
	free(sorted_pairs);
	free(extremes);
	free(path);
	lug_search_destroy(search);
	lugraph_destroy(tour);
}


// not optimal, just in order
void produce_optimal(int locations_length, location_in_use locations[])
{
	double total_dist = 0;
	for (int i = 0; i < locations_length; i++)
	{
		locations[i].visited = false;
	}

	for (int i = 0; i < locations_length - 1; i++)
	{
		total_dist += location_distance(&locations[i].loc, &locations[i+1].loc);
	}
	total_dist += location_distance(&locations[0].loc, &locations[locations_length - 1].loc);

	printf("-optimal        :%10.2lf", total_dist);
	for (int y = 0; y < locations_length; y++)
	{
		printf(" %s", locations[y].name);
	}
	printf(" %s", locations[0].name);
	putchar('\n');
}

/*
Open the file
Produces error messages
Reads the file into array of location_in_use structs
Read the rest of the argv arguments, executes the commands
*/
int main(int argc, char *argv[])
{

	//invalid input check
	if (argc < 2)
	{
		fprintf(stderr, "TSP: missing filename\n");
		return 1;
	}

	bool has_printed = false;

	FILE *file = fopen(argv[1], "r");

	if (file == NULL)
	{
		fprintf(stderr, "TSP: could not open %s\n", argv[1]);
		for (int i = 2; i < argc; i++)
		{
			printf("%s ", argv[i]);
			has_printed = true;
		}
		if (has_printed)
		{
			putchar('\n');
		}
		return 1;
	}

	int num_cities;
	fscanf(file, "%d", &num_cities);

	// filling up array locations with file data
	location_in_use locations[num_cities];

	for (int i = 0; i < num_cities; i++)
	{
		fscanf(file, "%s", locations[i].name);
		locations[i].visited = false;
	}

	for (int j = 0; j < num_cities; j++)
	{
		fscanf(file, "%lf %lf", &locations[j].loc.lat, &locations[j].loc.lon);
	}


	if (num_cities < 2)
	{
		fprintf(stderr, "TSP: too few cities\n");
		for (int i = 2; i < argc; i++)
		{
			printf("%s ", argv[i]);
			has_printed = true;
		}
		if (has_printed)
		{
			putchar('\n');
		}			
		return 1;
	}

	for (int i = 2; i < argc; i++)
	{

		if ((strcmp(argv[i], "-insert")) == 0)
		{
			i++;

			if (i == argc)
			{
				fprintf(stderr, "TSP: missing criterion\n");
				return 1;
			}

			if (((strcmp(argv[i], "nearest")) != 0)
				&& ((strcmp(argv[i], "farthest")) != 0))
			{
				fprintf(stderr, "TSP: invalid criterion %s\n", argv[i]);
				for (int j = i + 1; j < argc; j++)
				{
					printf("%s ", argv[j]);
					has_printed = true;
				}
				if (has_printed)
				{
					putchar('\n');
				}
				return 1;
			}
		}

		else if (((strcmp(argv[i], "-nearest")) != 0)
			&& ((strcmp(argv[i], "-optimal")) != 0) && ((strcmp(argv[i], "-greedy")) !=0))
		{
			fprintf(stderr, "TSP: invalid method %s\n", argv[i]);
			
			for (int j = i + 1; j < argc; j++)
			{
				printf("%s ", argv[j]);
				has_printed = true;
			}
			if (has_printed)
			{
				putchar('\n');
			}
			return 1;
		}
	}
	
	// reading command-line args, executing functions
	for (int i = 2; i < argc; i++)
	{

		if ((strcmp(argv[i], "-insert")) == 0)
		{
			i++;

			if ((strcmp(argv[i], "nearest")) == 0)
			{
				produce_ins_nearest(num_cities, locations);
			}
			else
			{
				produce_ins_farthest(num_cities, locations);
			}
		}
		else if ((strcmp(argv[i], "-nearest")) == 0)
		{
			produce_nearest(num_cities, locations);
		}
		else if ((strcmp(argv[i], "-greedy")) == 0)
		{
			produce_greedy(num_cities, locations);
		}
		else
		{
			// print original order
			produce_optimal(num_cities, locations);
		}
	}
}