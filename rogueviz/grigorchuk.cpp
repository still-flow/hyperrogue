// RogueViz - Grigorchuk group
// Copyright (C) 2011-2019 Zeno and Tehora Rogue, see 'hyper.cpp' for details

/** \file rogueviz/grigorchuk.cpp
 *  \brief Grigorchuk group
 *
 * This is a visualization of the Grigorchuk group. It is the first known group with
 * intermediate growth (i.e., superpolynomial and subexponential).
 *
 * The implementation is based on:
 *
 * Rostislav Grigorchuk, Igor Pak,
 * Groups of Intermediate Growth: an Introduction for Beginners
 * https://arxiv.org/pdf/math/0607384.pdf
 *
 * which presents the material in a simple way.
 *
 * This creates a map whose tiles correspond to the elements of the Grigorchuk group.
 * More precisely, the tiles correspond to the subgroup of index 2 generated by ac, ca, and b
 * (this is "more playable"). The three tiles adjacent to g are gb, gac, and gca.
 *
 * The 'lines' drawn split each tile into two halves, which correspond to the elements of the
 * actual Grigorchuk group (g and ga; ga is the one close to gac).
 *
 * Every element of the Grigorchuk group has finite order. Therefore, if you choose a specific
 * way of travelling (e.g. turn left, go, turn right, go) you will always eventually reach the
 * starting point.
 *
 * Command line options:
 *
 * -grigorchuk -- play on the Grigorchuk group
 * -grig-limit 100000 -canvas G -- color the tiles according to the distance from the starting point
 *   (i.e., the neutral element), the number is the number of tiles colored
 * -grig-nolines -- show no splitting lines (also can be switched in the experiments menu)
 * -grig-nolabels -- show no labels (also can be switched in the experiments menu)
 *
 */

#include "../hyper.h"

namespace grigorchuk {

using namespace hr;

typedef tuple<bool, string, string> splitter;

void add(string& s, char c) {
  if(s.size() == 0) s.push_back(c);
  else if(c == s.back()) s.pop_back();
  else if(c != 'a' && s.back() != 'a')
   s.back() = s.back() ^ c ^ 'd' ^ 'b' ^ 'c';
  else s.push_back(c);
  }

splitter split(string s) {
  bool swapped = false;
  string s0, s1;
  for(char c: s) {
    if(c == 'b') add(s0, swapped?'a':'c'), add(s1, swapped?'c':'a');
    if(c == 'c') add(s0, swapped?'a':'d'), add(s1, swapped?'d':'a');
    if(c == 'd') add(swapped ? s1 : s0, 'b');
    if(c == 'a') swapped = !swapped;
    }
  return splitter{swapped, s0, s1};
  }

splitter split_slow(string s) {
  bool swapped = false;
  string s0, s1;
  for(char c: s) {
    if(c == 'b') (s0 += swapped?'a':'c'), (s1 += swapped?'c':'a');
    if(c == 'c') (s0 += swapped?'a':'d'), (s1 += swapped?'d':'a');
    if(c == 'd') ((swapped ? s1 : s0) += 'b'), ((swapped ? s0 : s1) += '-');
    if(c == 'a') swapped = !swapped, s0 += '-', s1 += '-';
    }
  return splitter{swapped, s0, s1};
  }

string reduce(const string& x) {
  string res;
  for(char c: x) add(res, c);
  return res;
  }

#define Split(x) auto sw = split(x); auto swapped = get<0>(sw); auto s0 = get<1>(sw); auto s1 = get<2>(sw)

bool empt(const string& x) { 
  Split(x); // auto [swapped, s0, s1] = split(x);
  if(x == "") return true;
  if(x == "d") return false;
  if(swapped) return false;
  return empt(s0) && empt(s1);
  }

bool empt_slow(const string& x) { 
  Split(x); // auto [swapped, s0, s1] = split_slow(x);
  printf("%s -> %d %s %s\n", x.c_str(), swapped, s0.c_str(), s1.c_str());
  if(x == "") return true;
  if(x == "d") return false;
  if(swapped) return false;
  return empt(s0) && empt(s1);
  }

typedef const struct rep* prep;

struct rep {
  bool swapped;
  prep a0;
  prep a1;
  mutable char last;
  mutable bool visited = false;
  mutable int len;
  rep(bool s, prep a0, prep a1, char l, bool vis = false) : swapped(s), a0(a0), a1(a1), last(l), visited(vis), len(-1) {}
  };

bool operator < (const rep a, const rep b) {
  return tie(a.swapped, a.a0, a.a1) < tie(b.swapped, b.a0, b.a1);
  }

bool operator == (const rep a, const rep b) {
  return tie(a.swapped, a.a0, a.a1) == tie(b.swapped, b.a0, b.a1);
  }

rep grig_I = rep{false, &grig_I, &grig_I, 0, false};
extern rep grig_a, grig_b, grig_c, grig_d;

rep grig_a = rep{true, &grig_I, &grig_I, 'a', false};
rep grig_b = rep{false, &grig_a, &grig_c, 'b', false};
rep grig_c = rep{false, &grig_a, &grig_d, 'c', false};
rep grig_d = rep{false, &grig_I, &grig_b, 'd', false};

// (ab) c = a (a,c) (a,d) = a(a,c) (a,d) = 
map<rep, char> all_reps; //  = {grigid, &grigid};

prep lookup(rep x) {
  if(x == grig_I) return &grig_I;
  else if(x == grig_a) return &grig_a;
  else if(x == grig_b) return &grig_b;
  else if(x == grig_c) return &grig_c;
  else if(x == grig_d) return &grig_d;
  else if(all_reps.count(x)) return &(all_reps.find(x)->first);
  else return &(all_reps.emplace(x, 0).first->first);
  }

/*prep add_a(prep x) { 
  return lookup({!x->swapped, x->a0, x->a1, 'a'}); 
  }
prep add_d(prep x) { 
  if(x == &grig_I) return &grig_d;
  if(x == &grig_d) return &grig_I;
  return lookup({x->swapped, x->swapped?add_d(x->a0):x->a0, x->swapped?x->a1:add_d(x->a1), 'd'});
  }
prep add_c(prep x) { return lookup({x->swapped, (x->swapped?add_a:add_d)(x->a0), (x->swapped?add_d:add_a)(x->a1), 'c'}); }
prep add_b(prep x) { return lookup({x->swapped, (x->swapped?add_a:add_c)(x->a0), (x->swapped?add_c:add_a)(x->a1), 'b'}); }
*/

/* ostream& operator << (ostream& os, prep x) {
  if(x == &grig_I) return os << "I";
  // else if(x == &grig_a) return os << "a";
  else if(x == &grig_b) return os << "b";
  else if(x == &grig_c) return os << "c";
  else if(x == &grig_d) return os << "d";
  else {
    if(x->swapped) os << "a";
    os << "(" << x->a0 << "," << x->a1 << ")";
    return os;
    }
  } */

prep mul (prep x, prep y) {
  if(x == &grig_I) return y;
  if(y == &grig_I) return x;
  if(x == &grig_a && y == &grig_a) return &grig_I;
  if(x == &grig_b && y == &grig_b) return &grig_I;
  if(x == &grig_c && y == &grig_c) return &grig_I;
  if(x == &grig_d && y == &grig_d) return &grig_I;
  if(x == &grig_b && y == &grig_c) return &grig_d;
  if(x == &grig_c && y == &grig_b) return &grig_d;
  if(x == &grig_b && y == &grig_d) return &grig_c;
  if(x == &grig_d && y == &grig_b) return &grig_c;
  if(x == &grig_c && y == &grig_d) return &grig_b;
  if(x == &grig_d && y == &grig_c) return &grig_b;
  if(!y->swapped) return lookup(rep{x->swapped, mul(x->a0, y->a0), mul(x->a1, y->a1), y->last});
  else return lookup(rep{!x->swapped, mul(x->a1, y->a0), mul(x->a0, y->a1), y->last});
  }

string encode(string s) {
  if(s == "") return "I";
  else if( s == "d") return "d";
  else {
    Split(s); // auto [swapped, s0, s1] = split(s);
    return (swapped ? "a(" : "(") + encode(s0) + "," + encode(s1) + ")";
    }
  }

set<string> seen;

void addmore(const string& s, int more) {
  if(more == 0) {
    string sr = s;
    reverse(sr.begin(), sr.end());
    for(string q: seen) {
      string qo = q;
      for(char cr: sr) add(q, cr);
      if(empt(q)) { 
        // printf("%s = %s /%s\n", s.c_str(), qo.c_str(), sr.c_str()); 
        return; 
        }
      }
    seen.insert(s);
    // printf("%s\n", s.c_str());
    return;
    }
  for(char c: {'a', 'b', 'c', 'd'}) {
    string s1 = s;
    add(s1, c);
    if(isize(s1) != isize(s)+1) continue;
    addmore(s1, more-1);
    }
  }

string deform(prep x2) {
  string t = "";
  while(x2 != &grig_I) {
    if(x2->last == 'a') t += 'a', x2 = mul(x2, &grig_a);
    else if(x2->last == 'b') t += 'b', x2 = mul(x2, &grig_b);
    else if(x2->last == 'c') t += 'c', x2 = mul(x2, &grig_c);
    else if(x2->last == 'd') t += 'd', x2 = mul(x2, &grig_d);
    else if(x2->last == 'A') t += "ca", x2 = mul(mul(x2, &grig_c), &grig_a);
    else if(x2->last == 'C') t += "ac", x2 = mul(mul(x2, &grig_a), &grig_c);
    }
  reverse(t.begin(), t.end());
  return t;
  }

bool prepared = false;

prep ac, ca;

int grig_limit = 10000;

void prepare() {
  prepared = true;
  
  // rep* grigid = lookup(rep { false, NULL, NULL });
  
  ac = mul(&grig_a, &grig_c);
  ca = mul(&grig_c, &grig_a);
  // prep where = &grig_I;
  
  string s = "";

  
  /*
  for(int a=0; a<=32; a++) {
    // printf("%p -> %d %p %p\n", where, where->swapped, where->a0, where->a1);
    cout << where << " | " << encode(s) << "\n";
    where = ((a&1) ? add_a : add_b) (where);
    s += (a&1) ? 'a' : 'b';
    }
  
  string test = "ba";
  string pw = "";
  for(int i=0; i<=16; i++) {
    printf("%d: %d\n", i, empt(pw));
    pw += test;
    }
  */
  
  // printf("TEST %s\n", encode("bcd").c_str());

  vector<prep> all;
  auto visit = [&] (prep x, char l, int d) { 
    if(!x->visited) x->visited = true, x->last = l, all.push_back(x), x->len = d;
    };
  visit(&grig_I, 0, 0);
  int length = 0;
  int next = all.size();
  
  for(int i=0; i<grig_limit; i++) {
    if(i == next) {
      printf("%d @ %d\n", i/4, length);
      next = all.size(), length++;
      }
    prep x = all[i];
    if(1) {
      // printf("%s\n", deform(x).c_str());
      }
    visit(mul(x, &grig_b), 'b', x->len + 1);
    visit(mul(x, ac), 'A', x->len + 1);
    visit(mul(x, ca), 'C', x->len + 1);
    }
  
  prep test = &grig_b;
  test = mul(test, &grig_a);
  test = mul(test, &grig_d);
  test = mul(test, &grig_a);
  test = mul(test, &grig_d);
  printf("badad = %s\n", deform(test).c_str());
  }

bool view_labels = true, view_lines = true;

  }

namespace hr {

struct hrmap_grigorchuk : hrmap_standard {

  heptagon *origin;
  heptagon *getOrigin() override { return origin; }
  
  map<heptagon*, grigorchuk::prep> dec;
  map<grigorchuk::prep, heptagon*> enc;
  
  void gtie(heptagon* h, grigorchuk::prep p) {
    dec[h] = p;
    enc[p] = h;
    }

  hrmap_grigorchuk() {
    if(!grigorchuk::prepared) grigorchuk::prepare();
    origin = tailored_alloc<heptagon> (S7);
    origin->s = hsOrigin;
    origin->emeraldval = 0;
    origin->zebraval = 0;
    origin->fiftyval = 0;
    origin->fieldval = 0;
    origin->rval0 = origin->rval1 = 0;
    origin->cdata = NULL;
    origin->alt = NULL;
    origin->c7 = NULL;
    origin->distance = 0;
    origin->c7 = newCell(3, origin);
    gtie(origin, &grigorchuk::grig_I);
    }

  heptagon *create_step(heptagon *p, int d) override {
    auto pr = dec[p];
    // auto pr1 = pr;

    switch(d) {
      using namespace grigorchuk;
      case 0: pr = mul(mul(pr, &grig_a), &grig_c); break;
      case 1: pr = mul(mul(pr, &grig_c), &grig_a); break;
      case 2: pr = mul(pr, &grig_b); break;
      }
    
    heptagon *h;

    if(enc.count(pr)) {
      h = enc[pr];
      // println(hlog, deform(pr), "*", "acd"[d], " = ", deform(pr1));
      }
    else {
      pr->last = "ACb" [d];

      h = tailored_alloc<heptagon> (S7);
      h->s = hsOrigin;
      h->emeraldval = 0;
      h->zebraval = 0;
      h->fiftyval = 0;
      h->fieldval = 0;
      h->rval0 = h->rval1 = 0;
      h->cdata = NULL;
      h->alt = NULL;
      h->c7 = newCell(3, h);
      h->distance = p->distance + 1;
      gtie(h, pr);
      }
    
    h->c.connect(d == 2 ? 2 : 1-d, p, d, false);;
    return h;
    }
  
  void draw() override {
  
    dq::visited_by_matrix.clear();
    dq::enqueue_by_matrix(viewctr.at, actualV(viewctr, cview()));
    
    while(!dq::drawqueue.empty()) {      
      auto& p = dq::drawqueue.front();
      heptagon *h = get<0>(p);
      transmatrix V = get<1>(p);
      dynamicval<ld> b(band_shift, get<2>(p));
      bandfixer bf(V);
      dq::drawqueue.pop();
            
      cell *c = h->c7;
      if(!do_draw(c, V)) continue;
      
      if(grigorchuk::view_lines) queueline(V * ddspin(c, 2) * xpush0(cgi.tessf/2), V * ddspin(c, 2) * xpush0(-cgi.tessf), 0xFF00FFFF, 2);

      if(grigorchuk::view_labels) queuestr(V, 0.3, grigorchuk::deform(dec[c->master]), 0xFFFFFF);
      
      drawcell(c, V, 0, false);
      
      for(int i=0; i<3; i++) if(c->move(i))
        dq::enqueue_by_matrix(h->cmove(i), V * ddspin(c, i) * xpush(cgi.tessf) * iddspin(c->move(i), c->c.spin(i), M_PI));
      }
    }

  transmatrix relative_matrix(heptagon *h2, heptagon *h1) override {
    if(gmatrix0.count(h2->c7) && gmatrix0.count(h1->c7))
      return inverse(gmatrix0[h1->c7]) * gmatrix0[h2->c7];
    return Id;
    }

  transmatrix relative_matrix(cell *c2, cell *c1, const struct hyperpoint& hint) override {
    if(gmatrix0.count(c2) && gmatrix0.count(c1))
      return inverse(gmatrix0[c1]) * gmatrix0[c2];
    return Id;
    }
  };

eGeometry gGrigorchuk(eGeometry(-1));

void create_grigorchuk_geometry() {
  if(gGrigorchuk != eGeometry(-1)) return;
  ginf.push_back(ginf[gNormal]);
  gGrigorchuk = eGeometry(isize(ginf) - 1);
  variation = eVariation::pure;
  auto& gi = ginf[gGrigorchuk];
  gi.sides = 3;
  gi.vertex = 8;
  gi.flags = qANYQ | qEXPERIMENTAL;
  gi.tiling_name = "{3,8}";
  gi.quotient_name = "Grigorchuk";
  gi.shortname = "Grig";
  }

int args() {
  using namespace arg;
           
  if(0) ;
  else if(argis("-grig-limit")) {
    shift(); grigorchuk::grig_limit = argi();
    }

  else if(argis("-grigorchuk")) {

    PHASEFROM(3);
    
    stop_game();
    create_grigorchuk_geometry();    
    set_geometry(gGrigorchuk);
    set_variation(eVariation::pure);
    }
  
  else if(argis("-grig-nolines")) {
    grigorchuk::view_lines = false;
    }

  else if(argis("-grig-nolabels")) {
    grigorchuk::view_labels = false;
    }

  else return 1;
  return 0;
  }

auto hook = addHook(hooks_args, 100, args)
  + addHook(hooks_newmap, 100, [] { return geometry == gGrigorchuk ? new hrmap_grigorchuk : nullptr; })
  + addHook(patterns::hooks_generate_canvas, 100, [] (cell* c) {
    if(patterns::whichCanvas == 'G' && geometry == gGrigorchuk) 
      return 0x102008 * (1 + ((hrmap_grigorchuk*)currentmap)->dec[c->master]->len);
    return -1;
    })
  + addHook(dialog::hooks_display_dialog, 100, [] () {
    if(current_screen_cfunction() == showEuclideanMenu && geometry == gGrigorchuk) {
      dialog::addBoolItem_action(XLAT("Grigorchuk lines"), grigorchuk::view_lines, 'L'); 
      dialog::addBoolItem_action(XLAT("Grigorchuk labels"), grigorchuk::view_labels, 'M'); 
      }
    });
  
}

