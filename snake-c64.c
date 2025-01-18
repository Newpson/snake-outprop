#include <string.h> /* memset()  */
#include <stdbool.h> /* true, false */
#include <stdint.h>
#include <stdlib.h>

#define HEIGHT 25U*8U
#define WIDTH 40U*8U

#define UP 0x00U
#define DOWN 0x01U
#define LEFT 0x02U
#define RIGHT 0x03U

static uint8_t * const VICC1 = (uint8_t *) 0xD011;
static uint8_t * const VICM = (uint8_t *) 0xD018;
static const uint16_t invert[] = { DOWN, UP, RIGHT, LEFT, };

typedef uint8_t (*Map)[WIDTH/4];
static uint8_t * const PRA = (uint8_t *) 0xDC00;
static uint8_t (*colors)[40] = (uint8_t (*)[40]) 0x0400;
static uint8_t (*screen)[40][8] = (uint8_t (*)[40][8]) 0x2000;

typedef
struct game
{
	Map map;
	uint16_t length;
	uint16_t snake_x;
	uint16_t snake_y;
	uint16_t apple_x;
	uint16_t apple_y;
}
Game;

uint16_t direction_get(Map map, uint16_t x, uint16_t y)
{
	return (map[y][x/4] >> (2*(x%4))) & 0x03;
}

void direction_set(Map map, uint16_t x, uint16_t y, uint16_t d)
{
	uint16_t shift = 2*(x%4);
	uint16_t pos = x/4;
	map[y][pos] &= ~(0x03 << shift);
	map[y][pos] |= (d << shift);
}

void direction_move(uint16_t d, uint16_t *x, uint16_t *y)
{
	switch (d)
	{
		case UP:
			--(*y);
			if (*y > HEIGHT) *y = HEIGHT-1;
			break;
		case DOWN:
			++(*y);
			if (*y >= HEIGHT) *y = 0;
			break;
		case LEFT:
			--(*x);
			if (*x > WIDTH) *x = WIDTH-1;
			break;
		case RIGHT:
			++(*x);
			if (*x >= WIDTH) *x = 0;
			break;
	}
}

void direction_next(Map map, uint16_t *x, uint16_t *y)
{
	direction_move(direction_get(map, *x, *y), x, y);
}

void apple_split(uint16_t *apple_coord, uint16_t limit, Game *game)
{
	uint16_t mid;
	uint16_t topleft;
	uint16_t bottomright;
	uint16_t x;
	uint16_t y;

	uint16_t factor;
	uint16_t *snake_coord;

	*apple_coord = 0;
	if (apple_coord == &game->apple_x)
	{
		factor = HEIGHT;
		snake_coord = &x;
	}
	else
	{
		factor = 1U;
		snake_coord = &y;
	}

	while (*apple_coord != limit)
	{
		uint16_t i;
		x = game->snake_x;
		y = game->snake_y;
		mid = (*apple_coord + limit) / 2;
		topleft = 0;
		bottomright = 0;
		for (i = 0; i < game->length; ++i, direction_next(game->map, &x, &y))
		{
			if (*snake_coord >= *apple_coord && *snake_coord <= limit)
			{
				/* little overhead due to the generalization */
				if (apple_coord == &game->apple_y)
				{
					if (x != game->apple_x)
					{
						continue;
					}
				}

				if (*snake_coord <= mid)
				{
					++topleft;
				}
				else
				{
					++bottomright;
				}
			}
		}

		/* calculate number of free cells */
		topleft = (mid-*apple_coord+1)*factor - topleft;
		bottomright = (limit-mid)*factor - bottomright;

		if (rand()%(topleft+bottomright) + 1 > topleft)
		{
			*apple_coord = mid + 1;
		}
		else
		{
			limit = mid;
		}
	}
}

void game_apple_generate(Game *game)
{
	apple_split(&game->apple_x, WIDTH-1, game);
	apple_split(&game->apple_y, HEIGHT-1, game);
}

void game_init(Game *game)
{
	const unsigned char byte = (UP << 0) | (UP << 2) | (UP << 4) | (UP << 6);
	//game->map = malloc(HEIGHT*WIDTH/4 * sizeof(uint8_t));
	game->map = (uint8_t (*)[WIDTH/4]) 0x6180;
	memset(game->map, byte, HEIGHT*WIDTH/4);
	memset(screen, 0, 8000); /* clear */

	game->length = 100;
	game->snake_x = 0;
	game->snake_y = 0;

	game_apple_generate(game);
}

bool game_snake_move(Game *game, uint16_t d)
{
	direction_move(invert[d], &game->snake_x, &game->snake_y);
	direction_set(game->map, game->snake_x, game->snake_y, d);
	return false;
}

bool game_print_snake(Game *game)
{
	uint16_t i;
	uint16_t x = game->snake_x;
	uint16_t y = game->snake_y;
	screen[y/8][x/8][y%8] |= (1 << (7 - x%8)); /* draw head */
	for (i = 0; i < game->length; ++i)
	{
		direction_next(game->map, &x, &y);
		if (game->snake_x == x && game->snake_y == y)
		{
			return true;
		}
	}
	screen[y/8][x/8][y%8] &= ~(1 << (7 - x%8)); /* redraw tail */
	screen[game->apple_y/8][game->apple_x/8][game->apple_y%8] |= (1 << (7 - game->apple_x%8));
	return false;
}

int main(void)
{
	Game game;
//	uint16_t timer;
	uint16_t dir;
	uint16_t tdir;

	*VICC1 |= (1<<5); /* set hires bitmap mode */
	*VICM = (*VICM & ~(0x07<<1)) | 8; /* set character map memory address to 0x2000 */

	memset(colors, 0xF0, 1000); /* fill with white/black for fg/bg */

	game_init(&game);
	dir = game.map[0][0] & 0x03;
	for (;;)
	{
		if (game.length == WIDTH*HEIGHT)
		{
			// TODO
			break;
		}
		if (game_print_snake(&game))
		{
			// TODO
			break;
		}
//		for (timer = 0; timer < 4; ++timer)
//		{
			switch (*PRA & 0x0F)
			{
				case ~(1<<0) & 0x0F:
					if (dir != DOWN && dir != UP) tdir = DOWN; break;
				case ~(1<<1) & 0x0F:
					if (dir != UP && dir != DOWN) tdir = UP; break;
				case ~(1<<2) & 0x0F:
					if (dir != RIGHT && dir != LEFT) tdir = RIGHT; break;
				case ~(1<<3) & 0x0F:
					if (dir != LEFT && dir != RIGHT) tdir = LEFT; break;
			}
//		}

		dir = tdir;
		game_snake_move(&game, dir);

		if (game.snake_x == game.apple_x && game.snake_y == game.apple_y)
		{
			game_apple_generate(&game);
			++game.length;
		}
	}

	return 0;
}
