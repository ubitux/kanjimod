#!/usr/bin/env python2

import pprint, re, os

data = []
f = open('data/kanji', 'r')
lines = f.readlines()
order = []
img_counter = 0
for i, line in enumerate(lines):
    if line.startswith('<td><a href="'):
        m = re.search('<a href="([^"]*)">(.*)</a>', line)
        link, kan = m.group(1), m.group(2).strip()
        sym_id = int(os.path.basename(link).split('-', 1)[0])
        kan = kan.replace('&lt;','<').replace('&gt;','>')
        img_id = None
        if kan.startswith('<img'):
            m = re.search('src="([^"]*)"', kan)
            kan = m.group(1).strip()
            kan = '@' + os.path.basename(kan)
            img_id = img_counter
            img_counter += 1
        m = re.search('<td>(.*)</td>', lines[i + 1])
        eng = m.group(1).strip()
        data += [None] * (sym_id - len(data) + 1)
        datadict = {'kan': kan, 'eng': eng}
        if img_id is not None:
            datadict['imgid'] = img_id
        data[sym_id] = datadict
        order.append(sym_id)
f.close()

# we can not open the page from the extracted links in data/kanji because wget
# broke the file names badly, so we instead just use the page id to make them
# match
for page in os.listdir('data/kanjis'):
    kan = int(page.split('-', 1)[0])
    f = open('data/kanjis/' + page, 'r')
    comp_status = 'wait_open'
    for line in f.readlines():
        if comp_status == 'wait_open' and "class='span8'" in line:
            comp_status = 'opened'
            continue
        if comp_status == 'opened':
            if 'class="component"' in line:
                m = re.search('<a href="/kanji/([0-9]*)', line)
                comp = int(m.group(1))
                if data[kan].get('comp'): data[kan]['comp'].append(comp)
                else:                     data[kan]['comp'] = [comp]
            if line.startswith('</div>'):
                comp_status = 'closed'

print 'data = \\'
pprint.pprint(data)
print 'order = ' + str(order)
