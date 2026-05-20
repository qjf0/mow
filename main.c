/* mow @alpha 0.0.1
 * https://github.com/qjf0/mow
 *
 * Maintainer: qjf0 <qianjunfan0@outlook.com>
 *                  <qianjunfan0@gmail.com>
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct task {
	int date;
	char *title;

	char **comments;
	int comment_cnt;

	bool is_done;
};

struct tag {
	int id;
	char *name;

	struct task *tasks;
	int task_cnt;
};

/* global runtime */
struct {
	struct tag *tags;
	int tag_cnt;
} rt;

/* util funcs */

static int get_month_num(const char *m)
{
	const char *months[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	for (int i = 0; i < 12; i++) {
		if (strncmp(m, months[i], 3) == 0)
			return i + 1;
	}

	return 0;
}

static int parse_date_str(const char *s)
{
	char month_str[16];
	int day, year, m_num;

	if (sscanf(s, "%s %d, %d:", month_str, &day, &year) != 3)
		return 0;

	m_num = get_month_num(month_str);
	return year * 10000 + m_num * 100 + day;
}

static char *trim(char *s)
{
	char *end;

	while (isspace((unsigned char)*s))
		s++;

	if (*s == 0)
		return s;

	end = s + strlen(s) - 1;
	while (end > s && isspace((unsigned char)*end))
		end--;

	end[1] = '\0';
	return s;
}

static struct tag *find_tag(const char *name)
{
	int i;

	for (i = 0; i < rt.tag_cnt; i++) {
		if (strcmp(rt.tags[i].name, name) == 0)
			return &rt.tags[i];
	}
	return NULL;
}

/* core workflow funcs */
void version(void)
{
        printf("\n");
        printf("    ██▓█▓   ███  █░    █               mow @alpha 0.0.1\n");
        printf("    █▒█▒█  █▓ ▓█ ▓▒   ▒█                               \n");
        printf("    █ █ █  █   █ ░█ █ █▒                               \n");
        printf("    █ █ █  █   █  █▒█▒█                                \n");
        printf("    █ █ █  █   █  █████                                \n");
        printf("    █ █ █  █▓ ▓█  ▒█▒█▒     https://github.com/qjf0/mow\n");
        printf("    █ █ █   ███    █ █          Copyright (C) 2026 qjf0\n");
        printf("\n");
}

void parse(void)
{
	FILE *fp;
	char line[1024];
	int curr_date = 0;
	struct tag *curr_tag = NULL;
	struct task *curr_task = NULL;

	fp = fopen("test", "r");
	if (!fp) {
		perror("mow: cannot open file");
		return;
	}

	while (fgets(line, sizeof(line), fp)) {
		char *content;
		size_t len;

		line[strcspn(line, "\r\n")] = 0;
		content = trim(line);
		len = strlen(content);

		if (len == 0)
			continue;

		/* date line */
		if (strstr(content, "20") && content[len - 1] == ':') {
			curr_date = parse_date_str(content);
			curr_task = NULL;
			continue;
		}

		/* tag line */
		if (content[0] == '#') {
			char *tag_name = trim(content + 1);
			curr_tag = find_tag(tag_name);

			if (!curr_tag) {
				rt.tag_cnt++;
				rt.tags = realloc(rt.tags,
						  sizeof(struct tag) * rt.tag_cnt);
				curr_tag = &rt.tags[rt.tag_cnt - 1];
				curr_tag->id = rt.tag_cnt;
				curr_tag->name = strdup(tag_name);
				curr_tag->tasks = NULL;
				curr_tag->task_cnt = 0;
			}
			curr_task = NULL;
			continue;
		}

		/* task line */
		if ((strncmp(content, "x:", 2) == 0 ||
		     strncmp(content, "o:", 2) == 0) && curr_tag) {
			curr_tag->task_cnt++;
			curr_tag->tasks = realloc(curr_tag->tasks,
						  sizeof(struct task) * curr_tag->task_cnt);
			curr_task = &curr_tag->tasks[curr_tag->task_cnt - 1];

			curr_task->date = curr_date;
			curr_task->is_done = (content[0] == 'x');
			curr_task->title = strdup(trim(content + 2));
			curr_task->comments = NULL;
			curr_task->comment_cnt = 0;
			continue;
		}

		/* comment line */
		if (curr_task) {
			curr_task->comment_cnt++;
			curr_task->comments = realloc(curr_task->comments,
						      sizeof(char *) * curr_task->comment_cnt);
			curr_task->comments[curr_task->comment_cnt - 1] = strdup(content);
		}
	}

	fclose(fp);
}

void debug_output(void)
{
	for (int i = 0; i < rt.tag_cnt; i++) {
		printf("# %s\n", rt.tags[i].name);
		for (int j = 0; j < rt.tags[i].task_cnt; j++) {
			struct task *t = &rt.tags[i].tasks[j];
			printf("\t%s %s (Date: %d)\n",
			       t->is_done ? "[x]" : "[ ]", t->title, t->date);
			for (int k = 0; k < t->comment_cnt; k++)
				printf("\t\t%s\n", t->comments[k]);
		}
		printf("\n");
	}
}

void quit(void)
{
	for (int i = 0; i < rt.tag_cnt; i++) {
		for (int j = 0; j < rt.tags[i].task_cnt; j++) {
			struct task *t = &rt.tags[i].tasks[j];
			free(t->title);
			for (int k = 0; k < t->comment_cnt; k++)
				free(t->comments[k]);
			free(t->comments);
		}
		free(rt.tags[i].name);
		free(rt.tags[i].tasks);
	}
	free(rt.tags);
	rt.tags = NULL;
	rt.tag_cnt = 0;
}

int main(void)
{
	version();
	parse();
	debug_output();
	quit();

	return 0;
}
