/* $Id: layouts.c,v 1.2 2024/11/11 16:12:54 lecorbeau Exp $
 *
 * See LICENSE file for copyright and license details.
 */

static void bottomstack(Monitor *m);
static void deck(Monitor *m);
static void grid(Monitor *m);
static void monocle(Monitor *m);
static void rightmaster(Monitor *m);
static void tile(Monitor *);

void
bottomstack(struct Monitor *m)
{
	unsigned int i, n, w, mh, mx, tx, ns;
	struct Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0)
		return;
	if(n == 1){
		c = nexttiled(m->clients);
		resize(c, m->wx + m->gappx, m->wy + m->gappx, m->ww - 2 * c->bw - 2*m->gappx, m->wh - 2 * c->bw - 2*m->gappx, 0);
		return;
	}

	if (n > m->nmaster){
		mh = m->nmaster ? m->wh * m->mfact : m->gappx;
		ns = 2;
	}
	else{
		mh = m->wh;
		ns = 1;
	}
	for (i = 0, mx = tx = m->gappx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			w = (m->ww - mx) / (MIN(n, m->nmaster) - i) - m->gappx;
			resize(c, m->wx + mx, m->wy + m->gappx, w - 2*c->bw, mh - 2*c->bw - m->gappx*(5-ns)/2, 0);
			if(mx + WIDTH(c) + m->gappx < m->mw)
				mx += WIDTH(c) + m->gappx;
		} else {
			w = (m->ww - tx) / (n - i) - m->gappx;
			if(m->nmaster == 0)
				resize(c, m->wx + tx, m->wy + mh, w - (2*c->bw), m->wh - mh - 2*c->bw - m->gappx, False);
			else
				resize(c, m->wx + tx, m->wy + mh + m->gappx/ns, w - (2*c->bw), m->wh - mh - 2*c->bw - m->gappx*(5-ns)/2, False);
			if (tx + WIDTH(c) + m->gappx < m->mw)
				tx += WIDTH(c) + m->gappx;
		}
}

void
deck(Monitor *m) {
	unsigned int i, n, h, mw, my;
	Client *c;

	for(n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if(n == 0)
		return;

	if(n > m->nmaster) {
		mw = m->nmaster ? m->ww * m->mfact : 0;
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n - m->nmaster);
	}
	else
		mw = m->ww - m->gappx;
	for (i = my = 0, my = m->gappx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if(i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i) - m->gappx;
			resize(c, m->wx + m->gappx, m->wy + my, mw - (2*c->bw) - m->gappx, h - (2*c->bw), False);
			my += HEIGHT(c) + m->gappx;
		}
		else
			resize(c, m->wx + mw + m->gappx, m->wy + m->gappx, m->ww - mw - (2*c->bw) - 2*m->gappx, m->wh - (2*c->bw) - 2*m->gappx, False);
}

void
grid(Monitor *m) {
	unsigned int n, cols, rows, cn, rn, i, cx, cy, cw, ch;
	Client *c;

	for(n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++) ;
	if(n == 0)
		return;
	if(n == 1){
		c = nexttiled(m->clients);
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
		return;
	}

	/* grid dimensions */
	for(cols = 0; cols <= n/2; cols++)
		if(cols*cols >= n)
			break;
	if(n == 5) /* set layout against the general calculation: not 1:2:2, but 2:3 */
		cols = 2;
	rows = n/cols;

	/* window geometries */
	cw = cols ? m->ww / cols : m->ww;
	cn = 0; /* current column number */
	rn = 0; /* current row number */
	for(i = 0, c = nexttiled(m->clients); c; i++, c = nexttiled(c->next)) {
		if(i/rows + 1 > cols - n%cols)
			rows = n/cols + 1;
		ch = rows ? (m->wh-m->gappx) / rows : m->wh;
		cx = m->wx + m->gappx + cn*cw;
		cy = m->wy + m->gappx + rn*ch;
		resize(c, cx - cn*m->gappx/cols, cy, cw - 2 * (c->bw) - m->gappx - m->gappx/cols, ch - 2 * c->bw - m->gappx, False);
		rn++;
		if(rn >= rows) {
			rn = 0;
			cn++;
		}
	}
}

void
monocle(Monitor *m)
{
	unsigned int n = 0;
	Client *c;

	for (c = m->clients; c; c = c->next)
		if (ISVISIBLE(c))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->wx + m->gappx, m->wy + m->gappx, m->ww - 2 * c->bw - 2*m->gappx, m->wh - 2 * c->bw - 2*m->gappx, 0);
}

void
rightmaster(Monitor *m)
{
	unsigned int i, n, h, mw, my, ty;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0)
		return;

	if (n > m->nmaster)
		mw = m->nmaster ? m->ww * m->mfact : 0;
	else
		mw = m->ww;
	for (i = 0, my = ty = m->gappx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i) - m->gappx;
			resize(c, m->wx + m->gappx + m->ww - mw, m->wy + my, mw - (2*c->bw) - 2*m->gappx, h - (2*c->bw), 0);
			my += HEIGHT(c) + m->gappx;
		} else {
			h = (m->wh - ty) / (n - i) - m->gappx;
			resize(c, m->wx + m->gappx, m->wy + ty, m->ww - mw - (2*c->bw) - m->gappx, h - (2*c->bw), 0);
			ty += HEIGHT(c) + m->gappx;
		}
}

void
tile(Monitor *m)
{
	unsigned int i, n, h, mw, my, ty;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0)
		return;

	if (n > m->nmaster)
		mw = m->nmaster ? m->ww * m->mfact : 0;
	else
		mw = m->ww - m->gappx;
	for (i = 0, my = ty = m->gappx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i) - m->gappx;
			resize(c, m->wx + m->gappx, m->wy + my, mw - (2*c->bw) - m->gappx, h - (2*c->bw), 0);
			my += HEIGHT(c) + m->gappx;
		} else {
			h = (m->wh - ty) / (n - i) - m->gappx;
			resize(c, m->wx + mw + m->gappx, m->wy + ty, m->ww - mw - (2*c->bw) - 2*m->gappx, h - (2*c->bw), 0);
			ty += HEIGHT(c) + m->gappx;
		}
}



