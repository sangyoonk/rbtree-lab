#include "rbtree.h"

#include <stdlib.h>

rbtree *new_rbtree(void) {
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  p->root = NULL;
  p->nil = NULL;
  // TODO: initialize struct if needed
  return p;
}

void delete_rbtree(rbtree *t) {
  // TODO: reclaim the tree nodes's memory
  delete_node_lrv(t->root);
  free(t);
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  // TODO: implement insert
  node_t *parent_node = NULL;
  node_t *new_node = NULL;
  node_t *uncle = NULL;
  node_t *grand = NULL;

  int is_left = 0;

  if (t == NULL)
    return NULL;

  parent_node = find_parent_node(t, key, &is_left);

  // new_node 가 root 인 경우이다.
  if (parent_node == NULL) {
    new_node = create_node(t, NULL, is_left, key);
    return new_node;
  }

  new_node = create_node(t, parent_node, is_left, key);
  uncle = find_uncle(new_node);
  grand = find_grand(new_node);
  if (parent_node->color == RBTREE_RED) {
    if (uncle == NULL || uncle->color == RBTREE_BLACK) {
      insert_1_father_red_no_uncle_or_uncle_black(t, new_node);
      return new_node;
    }
    else {
      insert_2_father_red_uncle_red(t, new_node);
    }
  }
  return new_node;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  // TODO: implement find
  
  node_t *now_node = t->root;
  while (now_node != NULL) {
    if (now_node->key == key)
      break;
    else if (key < now_node->key) {
      now_node = now_node->left;
    }
    else {
      now_node = now_node->right;
    }
  }
  return now_node;
}

node_t *rbtree_min(const rbtree *t) {
  node_t *now_node = NULL;

  if (t == NULL || t->root == NULL)
    return NULL;

  now_node = t->root;
  while (now_node->left)
    now_node = now_node->left;

  return now_node;
}

node_t *rbtree_max(const rbtree *t) {
  node_t *now_node = NULL;

  if(t == NULL || t->root == NULL)
    return NULL;

  now_node = t->root;
  while (now_node->right)
    now_node = now_node->right;

  return now_node;
}

int rbtree_erase(rbtree *t, node_t *p) {

  //
  //  deleted_node : 삭제될 노드
  //  s : 삭제될 노드의 형제 노드
  //  x : 삭제될 노드 자리에 들어올 노드
  //
  node_t *deleted_node = p;
  node_t *s = NULL;
  node_t *x = NULL;

  if (t == NULL || t->root == NULL || p == NULL)
    return 0;

  //
  //  이것 이후에 deleted_node 는 NULL 이 아니게 된다.
  //
  if (p->left == NULL && p->right == NULL) {
    erase_0_has_no_children(t, deleted_node);
    free(deleted_node);
    return 0;
  }
  //
  //  p 대신에 삭제될 노드(deleted_node)를 구한다.
  //  그리고 그 자리를 대체할 x 를 구한다.
  //
  if (p->right) {
    deleted_node = p->right;
    while (deleted_node->left)
      deleted_node = deleted_node->left;

    p->key = deleted_node->key;
    s = find_sibling(deleted_node);
    x = deleted_node->right;
  }
  else {
    deleted_node = p->left;
    while (deleted_node->right)
      deleted_node = deleted_node->right;

    p->key = deleted_node->key;
    s = find_sibling(deleted_node);
    x = deleted_node->left;
  }
  //
  //  Case 0_삭제될 노드나 대체될 노드가 RED 일때
  //  이 이후로 deleted_node 와 x 의 color 는 무조건 BLACK 이다.
  //
  if (deleted_node->color == RBTREE_RED || 
        (x && x->color == RBTREE_RED)) {
    // printf("case_0\n");
    erase_0_m_is_red_or_ms_is_red(t, deleted_node, x);
    free(deleted_node);
    return 0;
  }
  //
  //  부모가 빨강, 형제 및 그의 자손들이 검정일 경우이다.
  //  부모를 검정, 형제를 빨강으로 만든다.
  //  이 이후로 p 가 RED 면 s 는 NULL 이 아니다.
  //
  if (deleted_node->parent->color == RBTREE_RED && 
      (s == NULL || 
        (s->color == RBTREE_BLACK &&
          (s->left == NULL || s->left->color == RBTREE_BLACK) &&
          (s->right == NULL || s->right->color == RBTREE_BLACK)
        )
      )
    ) {
    //  printf("case_1_1\n");
    exchange_m_x(deleted_node, x);
    erase_1_1_p_is_red_s_and_sl_sr_is_black(t, deleted_node->parent, s);
    free(deleted_node);
    return 0;
  }
  //
  //  형제 s 가 검정, 먼곳에 있는 조카가 빨강일 경우이다.
  //  부모를 기준으로 x 방향으로 회전한다.
  //
  if (deleted_node->parent->left == deleted_node) {
    if (s && s->color == RBTREE_BLACK && 
        s->right && s->right->color == RBTREE_RED) {
      // printf("case_x_2_l del = %d\n", deleted_node->key);
      exchange_m_x(deleted_node, x);
      erase_x_2_s_black_sr_is_out_and_red(t, deleted_node->parent, s);
      free(deleted_node);
      return 0;
    }
  }
  
  else {
    if (s && s->color == RBTREE_BLACK &&
        s->left && s->left->color == RBTREE_RED) {
      //  printf("case_x_2_r\n");
      exchange_m_x(deleted_node, x);
      erase_x_2_s_black_sl_is_out_and_red(t, deleted_node->parent, s);
      free(deleted_node);
      return 0;
    }
  }
  //
  //  형제 s 가 검정, 가까이에 있는 조카가 빨강일 경우이다.
  //  s 를 기준응로 x 의 바깥쪽으로 회전한다.
  //
  if (deleted_node->parent->left == deleted_node) {
    if (s && s->color == RBTREE_BLACK && 
        s->left && s->left->color == RBTREE_RED) {
      //  printf("case_x_3_l\n");
      exchange_m_x(deleted_node, x);
      erase_x_3_s_black_sl_is_in_and_red(t, deleted_node->parent, s);
      free(deleted_node);
      return 0;
    }
  } 
  else {
    if (s && s->color == RBTREE_BLACK &&
        s->right && s->right->color == RBTREE_RED) {
      //  printf("case_x_3_r\n");
      exchange_m_x(deleted_node, x);
      erase_x_3_s_black_sr_is_in_and_red(t, deleted_node->parent, s);
      free(deleted_node);
      return 0;
    }
  }
  //
  //  부모가 검정, 형제가 검정, 조카들도 검정일 경우다.
  //  형제의 색을 빨강으로 바꾼다.
  //  부모에 대해 재귀적으로 처리하라는데 어쩌란걸까...
  //  일단 보류
  //
  if (deleted_node->parent->color == RBTREE_BLACK && 
      (s == NULL || 
        (s->color == RBTREE_BLACK &&
          (s->left == NULL || s->left->color == RBTREE_BLACK) &&
          (s->right == NULL || s->right->color == RBTREE_BLACK)
        )
      )
    ) {
    exchange_m_x(deleted_node, x);
    erase_2_1_all_black(t, deleted_node->parent, s);
    free(deleted_node);
    return 0;
  }
  //
  //  부모가 검정, 형제는 빨강인 경우이다. 이 때 조카들은 전부 검정이다.
  //  부모의 색을 빨강, 형제의 색을 검정으로 바꾼다.
  //  그 뒤에 부모를 기준으로 x 쪽 발향으로 회전한다.
  //
  if (deleted_node->parent->left == deleted_node) {
    if (s && s->color == RBTREE_RED) {
      //  printf("case_2_4_l\n");
      exchange_m_x(deleted_node, x);
      erase_2_4_p_black_s_is_red_and_ps_right(t, deleted_node->parent, s);
      free(deleted_node);
      return 0;
    }
  }
  else {
    if (s && s->color == RBTREE_RED) {
      // printf("case_2_4_r\n");
      exchange_m_x(deleted_node, x);
      erase_2_4_p_black_s_is_red_and_ps_left(t, deleted_node->parent, s);
      free(deleted_node);
      return 0;
    }
  }
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  int now_size = 0;

  if (t == NULL || t->root == NULL)
    return 0;

  to_array_lvr(arr, t->root, n, &now_size);

  return now_size;
}


void test_lvr(node_t *node) {
  if (node == NULL)
    return;
  if (node->left)
    test_lvr(node->left);
  printf("LVR %d is %s\n", node->key, node->color == RBTREE_BLACK ? "BLACK" : "RED");
  if (node->right)
    test_lvr(node->right);
}

void test_vlr(node_t *node) {
  printf("VLR %d (%d) is %s\n", node->key, node->parent ? node->parent->key : 777, node->color == RBTREE_BLACK ? "BLACK" : "RED");
  if (node->left)
    test_vlr(node->left);
  if (node->right)
    test_vlr(node->right);
}

void test_lrv(node_t *node) {
  if (node->left)
    test_lrv(node->left);
  if (node->right)
    test_lrv(node->right);
  printf("LRV %d is %s\n", node->key, node->color == RBTREE_BLACK ? "BLACK" : "RED");
}


static void to_array_lvr(key_t *output_array, node_t *now_node, const size_t max_size, int *now_size) {

  if (max_size <= *now_size)
    return;

  if (now_node->left && (max_size > *now_size))
    to_array_lvr(output_array, now_node->left, max_size, now_size);

  if (max_size <= *now_size)
    return;
  output_array[*now_size] = now_node->key;
  *now_size += 1;

  if(now_node->right && (max_size > *now_size)) {
    to_array_lvr(output_array, now_node->right, max_size, now_size);
  }

}


static node_t *find_uncle(node_t *now_node) {
  node_t *grand = find_grand(now_node);

  if (grand == NULL)
    return NULL;

  if (now_node->parent == grand->left)
    return grand->right;

  return grand->left;
}

static node_t *find_grand(node_t *now_node) {
  if (now_node == NULL || now_node->parent == NULL || now_node->parent->parent == NULL)
    return NULL;

  return now_node->parent->parent;
}

static node_t *find_sibling(node_t *now_node) {
  if (now_node == NULL || now_node->parent == NULL)
    return NULL;

  if (now_node == now_node->parent->left)
    return now_node->parent->right;
  return now_node->parent->left;
}

//
//  입력 :
//    t : 트리
//    parent : node 를 넣어줄 parent
//    is_left : 왼쪽에 넣을지 여부를 결젖ㅇ
//    key : new node 의 key 값
//
//  출력 :
//    새로 만들어진 노드
//
static node_t *create_node(rbtree *t, node_t *parent, const int is_left, const key_t key) {
  node_t *new_node = (node_t *)malloc(sizeof(node_t));
  new_node->color = RBTREE_RED;
  new_node->key = key;
  new_node->parent = parent;
  new_node->left = NULL;
  new_node->right = NULL;

  if (parent) {
    if (is_left)
      parent->left = new_node;
    else
      parent->right = new_node;
  }
  else {
    new_node->color = RBTREE_BLACK;
    t->root = new_node;
  }
  
  return new_node;
}

//  입력 :
//    t 는 NULL 이 아니다.
//    또한 key 값이 같은 경우도 존재하지 않는다.
//
//  출력 :
//    key 값이 들어갈 곳의 부모노드를 리턴한다.
//    is_left 에 부모노드의 좌측인지 우측인지를 저장한다.
//
static node_t *find_parent_node(const rbtree *t, const key_t key, int *is_left) {
  
  node_t *now_node = t->root;

  while (now_node != NULL) {
    if (key < now_node->key) {
      *is_left = 1;

      if (now_node->left)
        now_node = now_node->left;
      else
        return now_node;
    }
    else {
      *is_left = 0;

      if (now_node->right)
        now_node = now_node->right;
      else
        return now_node;
    }
  }

  return now_node;
}


//
//  입력 :
//    father 랑 grand 가 NULL 일리는 없다.
//      father 는 RED 이며, 자연스럽게 grand 가 존재한다.
//    father 가 root 일리도 없다. (논리상 그렇다)
//
//  출력 :
//    일단 NULL 출력하게 해놨엉...
//
static node_t *insert_1_father_red_no_uncle_or_uncle_black(rbtree *t, node_t *child) {
  node_t *parent = child->parent;
  node_t *grand = find_grand(child);

  if (grand->left == parent && parent->left == child) {
    grand->left = parent->right;
    if (parent->right)
      parent->right->parent = grand;

    parent->right = grand;
    parent->parent = grand->parent;
    grand->parent = parent;

    parent->color = RBTREE_BLACK;
    grand->color = RBTREE_RED;

    if (t->root == grand)
      t->root = parent;
    else {
      if (grand == parent->parent->left)
        parent->parent->left = parent;
      else
        parent->parent->right = parent;
    }
  }
  else if (grand->left == parent && parent->right == child) {
    child->parent = grand->parent;
    grand->parent = child;
    parent->parent = child;
    parent->right = child->left;
    if (child->left)
      child->left->parent = parent;
    grand->left = child->right;
    if (child->right)
      child->right->parent = grand;
    child->right = grand;
    child->left = parent;

    child->color = RBTREE_BLACK;
    grand->color = RBTREE_RED;

    if (t->root == grand)
      t->root = child;
    else {
      if (grand == child->parent->left)
        child->parent->left = child;
      else
        child->parent->right = child;
    }
  }
  else if (grand->right == parent && parent->left == child) {
    child->parent = grand->parent;
    grand->right = child->left;
    if (child->left)
      child->left->parent = grand;
    parent->left = child->right;
    if (child->right)
      child->right->parent = parent;
    child->left = grand;
    child->right = parent;
    grand->parent = child;
    parent->parent = child;

    child->color = RBTREE_BLACK;
    grand->color = RBTREE_RED; 

    if (t->root == grand)
      t->root = child;
    else {
      if (grand == child->parent->left)
        child->parent->left = child;
      else
        child->parent->right = child;
    }
  }
  else {
    parent->parent = grand->parent;
    grand->right = parent->left;
    if (parent->left)
      parent->left->parent = grand;
    parent->left = grand;
    grand->parent = parent;

    parent->color = RBTREE_BLACK;
    grand->color = RBTREE_RED;

    if (t->root == grand)
      t->root = parent;
    else {
      if (grand == parent->parent->left)
        parent->parent->left = parent;
      else
        parent->parent->right = parent;
    }
  }

  return NULL;
}

static node_t *insert_2_father_red_uncle_red(rbtree *t, node_t *child) {
  node_t *grand = find_grand(child);
  node_t *grand_uncle = NULL;
  node_t *uncle = find_uncle(child);
  node_t *parent = child->parent;


  uncle->color = RBTREE_BLACK;
  parent->color = RBTREE_BLACK;
  grand->color = RBTREE_RED;

  if (t->root == grand) {
    grand->color = RBTREE_BLACK;
  }
  else if (grand->parent && grand->parent->color == RBTREE_RED) {
    grand_uncle = find_uncle(grand);

    if (grand_uncle == NULL || grand_uncle->color == RBTREE_BLACK)
      return insert_1_father_red_no_uncle_or_uncle_black(t, grand);
    else
      return insert_2_father_red_uncle_red(t, grand);
  }

  return child;
}

//
//  m->parent 가 NULL 이 아닌 상황에서만 부른다.
//
static void exchange_m_x(node_t *m, node_t *x) {
  node_t *p = m->parent;

  if (p->left == m)
    p->left = x;
  else
    p->right = x;
  if (x)
    x->parent = p;
}


static int erase_0_has_no_children(rbtree *t, node_t *now_node) {
  if (t->root == now_node)
    t->root = NULL;
  else {
    if (now_node->parent->left == now_node)
      now_node->parent->left = NULL;
    else
      now_node->parent->right = NULL;
  }

  return 0;
}

static int erase_0_m_is_red_or_ms_is_red(rbtree *t, node_t *m, node_t *x) {

  if (m->parent->left == m) {
    m->parent->left = x;
  }
  else {
    m->parent->right = x;
  }

  if (x) {
    x->parent = m->parent;
    x->color = RBTREE_BLACK;
  }

  return 0;
}

//
//    x 가 NULL 일수도 있어서 s 와 p를 받았다.
//
static int erase_1_1_p_is_red_s_and_sl_sr_is_black(rbtree *t, node_t *p, node_t *s) {
  p->color = RBTREE_BLACK;
  if (s)
    s->color = RBTREE_RED;

  return 0;
}

static int erase_x_2_s_black_sr_is_out_and_red(rbtree *t, node_t *p, node_t *s) {
  if (t->root == p) {
    t->root = s;
  }
  if (p->parent) {
    if (p->parent->left == p)
      p->parent->left = s;
    else
      p->parent->right = s;
  }

  s->parent = p->parent;
  p->parent = s;
  p->right = s->left;
  if (s->left)
    s->left->parent = p;
  s->left = p;

  s->color = p->color;
  p->color = RBTREE_BLACK;
  if (s->right)
    s->right->color = RBTREE_BLACK;
}

static int erase_x_2_s_black_sl_is_out_and_red(rbtree *t, node_t *p, node_t *s) {
  if (t->root == p)
    t->root = s;
  if (p->parent) {
    if (p->parent->left == p)
      p->parent->left = s;
    else
      p->parent->right = s;
  }

  s->parent = p->parent;
  p->parent = s;
  p->left = s->right;
  if (s->right)
    s->right->parent = p;
  s->right = p;

  s->color = p->color;
  p->color = RBTREE_BLACK;
  if (s->left)
    s->left->color = RBTREE_BLACK;
}

static int erase_x_3_s_black_sl_is_in_and_red(rbtree *t, node_t *p, node_t *s) {
  node_t *sl = s->left;

  p->right = sl;
  sl->parent = p;
  sl->color = RBTREE_BLACK;
  s->left = sl->right;
  if (sl->right)
    sl->right->parent = s;
  sl->right = s;
  s->parent = sl;
  s->color = RBTREE_RED;

  erase_x_2_s_black_sr_is_out_and_red(t, p, sl);  
}

static int erase_x_3_s_black_sr_is_in_and_red(rbtree *t, node_t *p, node_t *s) {
  node_t *sr = s->right;

  p->left = sr;
  sr->parent = p;
  sr->color = RBTREE_BLACK;
  s->right = sr->left;
  if (sr->left)
    sr->left->parent = s;
  sr->left = s;
  s->parent = sr;
  s->color = RBTREE_RED;

  erase_x_2_s_black_sl_is_out_and_red(t, p, sr);  
}

static int erase_2_1_all_black(rbtree *t, node_t *p, node_t *s) {
  node_t *ps = NULL;
  if (s)
    s->color = RBTREE_RED;
  
  if (t->root == p)
    return 0;


  ps = find_sibling(p);

  if (p->parent->color == RBTREE_RED && 
      (ps == NULL || 
        (ps->color == RBTREE_BLACK &&
          (ps->left == NULL || ps->left->color == RBTREE_BLACK) &&
          (ps->right == NULL || ps->right->color == RBTREE_BLACK)
        )
      )
    ) {
    erase_1_1_p_is_red_s_and_sl_sr_is_black(t, p->parent, ps);
    return 0;
  }

  //
  //  형제 s 가 검정, 먼곳에 있는 조카가 빨강일 경우이다.
  //  부모를 기준으로 x 방향으로 회전한다.
  //
  if (p->parent->left == p) {
    if (ps && ps->color == RBTREE_BLACK && 
        ps->right && ps->right->color == RBTREE_RED) {
      erase_x_2_s_black_sr_is_out_and_red(t, p->parent, ps);
      return 0;
    }
  }
  else {
    if (ps && ps->color == RBTREE_BLACK &&
        ps->left && ps->left->color == RBTREE_RED) {
      erase_x_2_s_black_sl_is_out_and_red(t, p->parent, ps);
      return 0;
    }
  }

  //
  //  형제 s 가 검정, 가까이에 있는 조카가 빨강일 경우이다.
  //  s 를 기준응로 x 의 바깥쪽으로 회전한다.
  //
  if (p->parent->left == p) {
    if (ps && ps->color == RBTREE_BLACK && 
        ps->left && ps->left->color == RBTREE_RED) {
      erase_x_3_s_black_sl_is_in_and_red(t, p->parent, ps);
      return 0;
    }
  } 
  else {
    if (ps && ps->color == RBTREE_BLACK &&
        ps->right && ps->right->color == RBTREE_RED) {
      erase_x_3_s_black_sr_is_in_and_red(t, p->parent, ps);
      return 0;
    }
  }

  //
  //  부모가 검정, 형제가 검정, 조카들도 검정일 경우다.
  //  형제의 색을 빨강으로 바꾼다.
  //  부모에 대해 재귀적으로 처리하라는데 어쩌란걸까...
  //  일단 보류
  //
  if (p->parent->color == RBTREE_BLACK && 
      (ps == NULL || 
        (ps->color == RBTREE_BLACK &&
          (ps->left == NULL || ps->left->color == RBTREE_BLACK) &&
          (ps->right == NULL || ps->right->color == RBTREE_BLACK)
        )
      )
    ) {
    erase_2_1_all_black(t, p->parent, ps);
    return 0;
  }
  if (p->parent->left == p) {
    if (ps && ps->color == RBTREE_RED) {
      erase_2_4_p_black_s_is_red_and_ps_right(t, p->parent, ps);
      return 0;
    }
  }
  else {
    if (ps && ps->color == RBTREE_RED) {
      erase_2_4_p_black_s_is_red_and_ps_left(t, p->parent, ps);
      return 0;
    }
  }
}

static int erase_2_4_p_black_s_is_red_and_ps_right(rbtree *t, node_t *p, node_t *s) {
  node_t *sl = s->left;

  if (t->root == p) {
    t->root = s;
  }
  if (p->parent) {
    if (p->parent->left == p)
      p->parent->left = s;
    else
      p->parent->right = s;
  }

  s->parent = p->parent;
  p->parent = s;
  p->right = sl;
  if (sl)
    sl->parent = p;
  s->left = p;

  s->color = RBTREE_BLACK;
  p->color = RBTREE_RED;

  if (sl == NULL || 
      (sl->color == RBTREE_BLACK &&
        (sl->left == NULL || sl->left->color == RBTREE_BLACK) &&
        (sl->right == NULL || sl->right->color == RBTREE_BLACK)
      )
    ) {
    erase_1_1_p_is_red_s_and_sl_sr_is_black(t, p, sl);
    return 0;
  }

  if (sl && sl->color == RBTREE_BLACK && 
      sl->right && sl->right->color == RBTREE_RED) {
    erase_x_2_s_black_sr_is_out_and_red(t, p, sl);
    return 0;
  }

  if (sl && sl->color == RBTREE_BLACK && 
      sl->left && sl->left->color == RBTREE_RED) {
    erase_x_3_s_black_sl_is_in_and_red(t, p, sl);
    return 0;
  }
}

static int erase_2_4_p_black_s_is_red_and_ps_left(rbtree *t, node_t *p, node_t *s) {
  node_t *sr = s->right;
  if (t->root == p) {
    t->root = s;
  }
  if (p->parent) {
    if (p->parent->left == p)
      p->parent->left = s;
    else
      p->parent->right = s;
  }

  s->parent = p->parent;
  p->parent = s;
  p->left = sr;
  if (sr)
    sr->parent = p;
  s->right = p;

  s->color = RBTREE_BLACK;
  p->color = RBTREE_RED;

  if (sr == NULL || 
      (sr->color == RBTREE_BLACK &&
        (sr->left == NULL || sr->left->color == RBTREE_BLACK) &&
        (sr->right == NULL || sr->right->color == RBTREE_BLACK)
      )
    ) {
    erase_1_1_p_is_red_s_and_sl_sr_is_black(t, p, sr);
    return 0;
  }

  if (sr && sr->color == RBTREE_BLACK && 
      sr->left && sr->left->color == RBTREE_RED) {
    erase_x_2_s_black_sl_is_out_and_red(t, p, sr);
    return 0;
  }

  if (sr && sr->color == RBTREE_BLACK && 
      sr->right && sr->right->color == RBTREE_RED) {
    erase_x_3_s_black_sr_is_in_and_red(t, p, sr);
    return 0;
  }
}


//
//  p_col, s_col, sl_col, sr_col
//
int check_erase_condition(rbtree *t, node_t *now_node, color_t p_col, color_t s_col, color_t sl_col, color_t sr_col) {
  node_t *p = NULL;
  node_t *s = NULL;
  node_t *sl = NULL;
  node_t *sr = NULL;
  
  if (t == NULL || t->root == NULL)
    return 0;

  if (now_node == NULL)
    return 0;

  p = now_node->parent;
  s = find_sibling(now_node);

  if (p == NULL && (p_col == RBTREE_RED || s_col == RBTREE_RED || sl_col == RBTREE_RED || sr_col == RBTREE_RED))
    return 0;
  else if (p == NULL)
    return 1;

  if (s == NULL && (s_col == RBTREE_RED || sl_col == RBTREE_RED || sr_col == RBTREE_RED))
    return 0;
  else if (p->color == p_col && s == NULL && s_col == RBTREE_BLACK)
    return 1;
  else if (p->color != p_col)
    return 0;

  //  이제 p 나 s 는 NULL 이 아니다.
  sl = s->left;
  sr = s->right;

  if (p->color == p_col && 
      s->color == s_col &&
      ((sl == NULL && sl_col == RBTREE_BLACK) || (sl && sl->color == sl_col)) &&
      ((sr == NULL && sr_col == RBTREE_BLACK) || (sr && sr->color == sr_col)) )
    return 1;

  return 0;
  
}


static void delete_node_lrv(node_t *now_node) {
  if (now_node == NULL)
    return;

  if (now_node->left)
    delete_node_lrv(now_node->left);
  if (now_node->right)
    delete_node_lrv(now_node->right);
  free(now_node);
}


