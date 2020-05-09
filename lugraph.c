#define _GNU_SOURCE

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "lugraph.h"

struct lugraph
{
  int n;          // the number of vertices
  int *list_size; // the size of each adjacency list
  int *list_cap;  // the capacity of each adjacency list
  int **adj;      // the adjacency lists
};

struct lug_search
{
  const lugraph *g;
  int from;
  int *dist;
  int *pred;
  int *visited;
};

#define LUGRAPH_ADJ_LIST_INITIAL_CAPACITY 4

/**
 * Resizes the adjacency list for the given vertex in the given graph.
 *
 * @param g a pointer to a directed graph
 * @param from the index of a vertex in that graph
 */
void lugraph_list_embiggen(lugraph *g, int from);

/**
 * Prepares a search result for the given graph starting from the given
 * vertex.  It is the responsibility of the caller to destroy the result.
 *
 * @param g a pointer to a directed graph
 * @param from the index of a vertex in that graph
 * @return a pointer to a search result
 */
lug_search *lug_search_create(const lugraph *g, int from);

/**
 * Adds the to vertex to the from vertex's adjacency list.
 *
 * @param g a pointer to a graph, non-NULL
 * @param from the index of a vertex in g
 * @param to the index of a vertex in g
 */
void lugraph_add_half_edge(lugraph *g, int from, int to);

lugraph *lugraph_create(int n)
{
  if (n < 1)
  {
      return NULL;
  }

  lugraph *g = malloc(sizeof(lugraph));
  if (g != NULL)
  {
      g->n = n;
      g->list_size = malloc(sizeof(int) * n);
      g->list_cap = malloc(sizeof(int) * n);
      g->adj = malloc(sizeof(int *) * n);

      if (g->list_size == NULL || g->list_cap == NULL || g->adj == NULL)
	    {
	       free(g->list_size);
	       free(g->list_cap);
	       free(g->adj);
	       free(g);

	       return NULL;
	     }

      for (int i = 0; i < n; i++)
    	{
    	  g->list_size[i] = 0;
    	  g->adj[i] = malloc(sizeof(int) * LUGRAPH_ADJ_LIST_INITIAL_CAPACITY);
    	  g->list_cap[i] = g->adj[i] != NULL ? LUGRAPH_ADJ_LIST_INITIAL_CAPACITY : 0;
    	}
  }

  return g;
}

int lugraph_size(const lugraph *g)
{
  if (g != NULL)
    {
      return g->n;
    }
  else
    {
      return 0;
    }
}

void lugraph_list_embiggen(lugraph *g, int from)
{
  if (g->list_cap[from] != 0)
    {
      int *bigger = realloc(g->adj[from], sizeof(int) * g->list_cap[from] * 2);
      if (bigger != NULL)
	{
	  g->adj[from] = bigger;
	  g->list_cap[from] = g->list_cap[from] * 2;
	}
    }
}

void lugraph_add_edge(lugraph *g, int from, int to)
{
  if (g != NULL && from >= 0 && to >= 0 && from < g->n && to < g->n && from != to)
    {
      lugraph_add_half_edge(g, from, to);
      lugraph_add_half_edge(g, to, from);
    }
}

void lugraph_add_half_edge(lugraph *g, int from, int to)
{
  if (g->list_size[from] == g->list_cap[from])
    {
      lugraph_list_embiggen(g, from);
    }

  if (g->list_size[from] < g->list_cap[from])
    {
      g->adj[from][g->list_size[from]++] = to;
    }
}

bool lugraph_has_edge(const lugraph *g, int from, int to)
{
  if (g != NULL && from >= 0 && to >= 0 && from < g->n && to < g->n && from != to)
    {
      int i = 0;
      while (i < g->list_size[from] && g->adj[from][i] != to)
	{
	  i++;
	}
      return i < g->list_size[from];
    }
  else
    {
      return false;
    }
}

int lugraph_degree(const lugraph *g, int v)
{
  if (g != NULL && v >= 0 && v < g->n)
    {
      return g->list_size[v];
    }
  else
    {
      return -1;
    }
}

void lugraph_destroy(lugraph *g)
{
  if (g != NULL)
    {
      for (int i = 0; i < g->n; i++)
  {
    free(g->adj[i]);
  }
      free(g->adj);
      free(g->list_cap);
      free(g->list_size);
      free(g);
    }
}

lug_search *lug_search_create(const lugraph *g, int from)
{
  if (g != NULL && from >= 0 && from < g->n)
    {
      lug_search *s = malloc(sizeof(lug_search));

      if (s != NULL)
  {
    s->g = g;
    s->from = from;
    s->dist = malloc(sizeof(int) * g->n);
    s->pred = malloc(sizeof(int) * g->n);
    s->visited = malloc(sizeof(int) * g->n);

    if (s->dist != NULL && s->pred != NULL && s->visited != NULL)
      {
        for (int i = 0; i < g->n; i++)
    {
      s->dist[i] = -1;
      s->pred[i] = -1;
      s->visited[i] = 0;
    }
      }
    else
      {
        free(s->pred);
        free(s->dist);
        free(s->visited);
        free(s);
        return NULL;
      }
  }

      return s;
    }
  else
    {
      return NULL;
    }
}

void lug_search_destroy(lug_search *s)
{
  if (s != NULL)
    {
      free(s->dist);
      free(s->pred);
      free(s->visited);
      free(s);
    }
}

lug_search *lugraph_bfs(const lugraph *g, int from)
{
    if (from < 0 || from >= g->n || g == NULL)
    {
        return NULL;
    }

    lug_search *search = lug_search_create(g, from);
    if (search == NULL)
    {
        exit(1);
    }

    int front = 0;
    int back = 0;
    int *queue = malloc(g->n * sizeof(int));
    if (queue == NULL)
    {
        free(search);
        exit(1);
    }

    void enqueue(int pred, int x)
    {
        queue[back] = x;
        back++;
        search->visited[x] = 1;
        search->pred[x] = pred;
        search->dist[x] = (search->pred[x] == -1 ? 0 : search->dist[search->pred[x]] + 1);
    }

    enqueue(-1, from);

    void dequeue(int adj_list_length, int *adj_list, int x)
    {
        front++;
        for (int i = 0; i < adj_list_length; i++)
        {
            if (search->visited[adj_list[i]] == 0)
            {
                enqueue(x, adj_list[i]);
            }
        }
    }

    while (front < back)
    {
        dequeue(g->list_size[queue[front]], g->adj[queue[front]], queue[front]);
    }
 
    free(queue);
    return search;
}

bool lugraph_connected(const lugraph *g, int from, int to)
{
    if (from < 0 || from >= g->n || to < 0 || to >= g->n || g == NULL)
    {
        return false;
    }

    lug_search *bfs = lugraph_bfs(g, from);
    if (bfs == NULL)
    {
        exit(1);
    }

    int pred = bfs->pred[to];
    while (pred != -1)
    {
        if (pred == from)
        {
            lug_search_destroy(bfs);
            return true;
        }
        else
        {
            pred = bfs->pred[pred];
        }
    }
    lug_search_destroy(bfs);
    return false;
}



int lug_search_distance(const lug_search *s, int to)
{
    if (to < 0 || to >= s->g->n || s == NULL)
    {
        return -1;
    }
    else
    {
        return (s->dist[to]);
    } 
}

int *lug_search_path(const lug_search *s, int to, int *len)
{
    *len = lug_search_distance(s, to);
    if (*len != -1)
    {
        int *path = malloc(sizeof(int) * (*len + 1));
        if (path == NULL)
        {
            exit(1);
        }
        path[0] = s->from;
        path[*len] = to; 
        int pred = s->pred[to];
        for (int i = *len - 1; i > 0; i--)
        {
            path[i] = pred;
            pred = s->pred[pred];
        }

        return path;
    }
    else // invalid input
    {
        return NULL;
    }
}