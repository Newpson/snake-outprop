#include <stdio.h> /* printf() */
#include <stdlib.h> /* EXIT_SUCCESS, random(), srand() */
#include <stdint.h> /* uint8_t */
#include <assert.h> /* assert() */
#include <stddef.h> /* NULL */
#include <string.h> /* memset()  */
#include <stdbool.h> /* true, false */
#include <time.h> /* time() */

#define HEIGHT 4U
#if HEIGHT <= 0
#error HEIGHT must be positive
#endif

#define WIDTH 4U
#if WIDTH <= 0
#error WIDTH must be positive
#endif
#if WIDTH % 4 != 0
#error WIDTH must be divisable by 4
#endif

#define CELLS WIDTH*HEIGHT

typedef unsigned int uint;
typedef uint8_t Map[HEIGHT][WIDTH/4];
typedef char Buffer[HEIGHT][WIDTH];

typedef
enum direction
{
	UP = 0x00,
	DOWN = 0x01,
	LEFT = 0x02,
	RIGHT = 0x03,
}
Direction;

Direction invert[] =
{
	[UP] = DOWN,
	[DOWN] = UP,
	[LEFT] = RIGHT,
	[RIGHT] = LEFT,
};

typedef
struct game
{
	Map map;
	uint length;
	uint snake_x;
	uint snake_y;
	uint apple_x;
	uint apple_y;
	Buffer buffer;
}
Game;

Direction direction_get(Map map, uint x, uint y)
{
	return (map[y][x/4] >> (2*(x%4))) & 0x03;
}

void direction_set(Map map, uint x, uint y, Direction d)
{
	uint shift = 2*(x%4);
	uint pos = x/4;
	map[y][pos] &= ~(0x03 << shift);
	map[y][pos] |= (d << shift);
}

void direction_move(Direction d, uint *x, uint *y)
{
	switch (d)
	{
		case UP:
			--(*y);
			break;
		case DOWN:
			++(*y);
			break;
		case LEFT:
			--(*x);
			break;
		case RIGHT:
			++(*x);
			break;
	}
}

void direction_next(Map map, uint *x, uint *y)
{
	direction_move(direction_get(map, *x, *y), x, y);
}

void apple_split(uint *apple_coord, uint limit, Game *game)
{
	*apple_coord = 0;
	uint mid;
	uint topleft;
	uint bottomright;
	uint x;
	uint y;

	/* generalization of the algorithm */
	uint factor;
	uint *snake_coord;
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
		topleft = 0;
		bottomright = 0;
		x = game->snake_x;
		y = game->snake_y;
		mid = (*apple_coord + limit) / 2;
		for (uint i = 0; i < game->length; ++i, direction_next(game->map, &x, &y))
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

		if (random()%(topleft+bottomright) + 1 > topleft)
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

// Verbose version of apple generation algorithm (tend to be more human readable)
void game_apple_generate_verbose(Game *game)
{
	assert(game != NULL);

	uint left = 0;
	uint right = WIDTH-1;
	uint top = 0;
	uint bottom = HEIGHT-1;
	uint mid;

	uint left_count;
	uint right_count;
	uint top_count;
	uint bottom_count;
	uint x;
	uint y;

	while (left != right)
	{
		left_count = 0;
		right_count = 0;
		x = game->snake_x;
		y = game->snake_y;
		mid = (left + right) / 2;
		for (uint i = 0; i < game->length; ++i)
		{
			if (x >= left && x <= right)
			{
				if (x <= mid)
				{
					++left_count;
				}
				else /* (x > mid) */
				{
					++right_count;
				}
			}
			direction_next(game->map, &x, &y);
		}

		/* calculate number of free cells */
		left_count = (mid-left+1)*HEIGHT - left_count;
		right_count = (right-mid)*HEIGHT - right_count;

		if (random()%(left_count+right_count) + 1 > left_count)
		{
			left = mid + 1;
		}
		else
		{
			right = mid;
		}
	}

	game->apple_x = left;

	while (top != bottom)
	{
		top_count = 0;
		bottom_count = 0;
		x = game->snake_x;
		y = game->snake_y;
		mid = (top + bottom) / 2;
		for (uint i = 0; i < game->length; ++i)
		{
			if (x == game->apple_x && y >= top && y <= bottom)
			{
				if (y <= mid)
				{
					++top_count;
				}
				else /* (x > mid) */
				{
					++bottom_count;
				}
			}
			direction_next(game->map, &x, &y);
		}

		/* calculate number of free cells */
		top_count = mid-top+1 - top_count;
		bottom_count = bottom-mid - bottom_count;

		if (random()%(top_count+bottom_count) + 1 > top_count)
		{
			top = mid + 1;
		}
		else
		{
			bottom = mid;
		}
	}

	game->apple_y = top;

//	game->apple_x = random() % WIDTH;
//	game->apple_y = random() % HEIGHT;
}

void game_init(Game *game)
{
	assert(game != NULL);

	const uint8_t byte = (UP << 0) | (UP << 2) | (UP << 4) | (UP << 6);
	memset(game->map, byte, HEIGHT*WIDTH/4);
	memset(game->buffer, '.', HEIGHT*WIDTH);

	game->length = 2;
	game->snake_x = WIDTH / 2;
	game->snake_y = HEIGHT / 2;

	game_apple_generate(game);
}

void game_print(Game *game)
{
	assert(game != NULL);

	for (uint i = 0; i < HEIGHT; ++i)
	{
		for (uint j = 0; j < WIDTH/4; ++j)
		{
			printf("%08b ", game->map[i][j]);
		}
		printf("\n");
	}

	printf("\n");

	for (uint i = 0; i < HEIGHT; ++i)
	{
		for (uint j = 0; j < WIDTH; ++j)
		{
			Direction d = direction_get(game->map, j, i);
			printf("%s ", d == UP ? "^" : d == DOWN ? "v" : d == LEFT ? "<" : d == RIGHT ? ">" : "_");
		}
		printf("\n");
	}
}

bool game_snake_bump(Game *game)
{
	uint x = game->snake_x;
	uint y = game->snake_y;

	/* avoid SEGV (split checks of self-collision and walls) */
//	if (x >= WIDTH || y >= HEIGHT)
//	{
//		return true;
//	}

	for (uint i = 0; i < game->length; ++i)
	{
		direction_next(game->map, &x, &y);
		if (game->snake_x == x && game->snake_y == y)
		{
			return true;
		}
	}
	return false;
}

bool game_snake_move(Game *game, Direction d)
{
	direction_move(invert[d], &game->snake_x, &game->snake_y);
	direction_set(game->map, game->snake_x, game->snake_y, d);
	return false;
}

void game_print_snake(Game *game)
{
	assert(game != NULL);
	
	uint x = game->snake_x;
	uint y = game->snake_y;
	game->buffer[y][x] = 'O'; /* redraw head */
	for (uint i = 0; i < game->length; ++i)
	{
		direction_next(game->map, &x, &y);
	}
	game->buffer[y][x] = '.'; /* redraw tail */
	game->buffer[game->apple_y][game->apple_x] = 'X';

	for (uint i = 0; i < HEIGHT; ++i)
	{
		for (uint j = 0; j < WIDTH; ++j)
		{
			printf("%c ", game->buffer[i][j]);
		}
		printf("\n");
	}
}

int main(void)
{
	srand(time(NULL));

	Game game;
	game_init(&game);

	char input = 0;

	while (input != 'q')
	{
		system("clear");

		if (game.length == WIDTH*HEIGHT)
		{
			printf("Win\n");
			break;
		}
		game_print_snake(&game);
		printf("\n");
		game_print(&game);
		input = getc(stdin);
		switch (input)
		{
			case 'k':
				game_snake_move(&game, DOWN);
				break;
			case 'j':
				game_snake_move(&game, UP);
				break;
			case 'h':
				game_snake_move(&game, RIGHT);
				break;
			case 'l':
				game_snake_move(&game, LEFT);
				break;
		}

		if (game_snake_bump(&game))
		{
			break;
		}

		if (game.snake_x == game.apple_x && game.snake_y == game.apple_y)
		{
			game_apple_generate(&game);
			++game.length;
		}
	}

	return EXIT_SUCCESS;
}
