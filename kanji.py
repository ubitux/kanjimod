#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import sys, xmlrpclib, time, kandb

added = {}

def kan_add(kan_idx):
    added[kan_idx] = vrtx = G.new_vertex()
    #G.set_vertex_attribute(vrtx, 'shape', 'sphere')
    #G.set_vertex_attribute(vrtx, 'size', '0.5')
    kanji = kandb.data[kan_idx]['kan']
    label = '[%d]' % kandb.data[kan_idx]['imgid'] if kanji[0] == '@' else kanji
    label += ' ' + kandb.data[kan_idx]['eng']
    #label = "好foo'baçåpqqpt\",œ☭|ABC"
    G.set_vertex_attribute(vrtx, 'label', label + '@')
    if 'imgid' in kandb.data[kan_idx]:
        G.set_vertex_attribute(vrtx, 'fontcolor', '#00ff00')
    elif 'comp' not in kandb.data[kan_idx]:
        G.set_vertex_attribute(vrtx, 'fontcolor', '#ff0000')

server = xmlrpclib.Server('http://localhost:20738/RPC2')
G = server.ubigraph
G.clear()

if len(sys.argv) > 1 and sys.argv[1] == 'clear':
    sys.exit(0)

for kan_id in kandb.order:
    print '%d: %s (%s)' % (kan_id, kandb.data[kan_id]['kan'], kandb.data[kan_id]['eng'])
    kan_add(kan_id)
    for comp in kandb.data[kan_id].get('comp', []):
        print '  → %s (%s)' % (kandb.data[comp]['kan'], kandb.data[comp]['eng'])
        if comp != kan_id:
            if comp not in added:
                kan_add(comp)
            edge = G.new_edge(added[comp], added[kan_id])
            G.set_edge_attribute(edge, 'spline', 'true')
            #G.set_edge_attribute(edge, 'arrow', 'true')
    time.sleep(1)
    #raw_input()
