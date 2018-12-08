#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <bsd/stdlib.h>

#include "file.h"
#include "utils.h"

typedef enum
{
    EVENT_BEGIN_SHIFT = 0,
    EVENT_FALL_ASLEEP,
    EVENT_WAKEUP,
} event_type_t;

typedef struct event
{
    uint32_t guard_id;
    event_type_t type;
    struct tm datetime;    
    time_t time;
} event_t;

typedef struct guard_stats
{
    uint32_t guard_id;
    uint32_t most_seen_minute;
    uint32_t minutes[60];
    uint32_t total_minutes_asleep;
} guard_stats_t;

typedef struct minute_stats
{
    uint32_t guard_id;
    uint32_t times_asleep;
} minute_stats_t;

uint32_t highest_guard_id = 0;

bool parse_event(line_t *line)
{
    event_t *event = calloc(1, sizeof(event_t));
    char *s = line_string(line);
    char *event_str = NULL;
    sscanf(s, "[%d-%d-%d %02d:%02d] ", &event->datetime.tm_year, &event->datetime.tm_mon, &event->datetime.tm_mday, &event->datetime.tm_hour, &event->datetime.tm_min);
    event->datetime.tm_isdst = 1;
    event->datetime.tm_year = 2018 - 1900;
    event->datetime.tm_mon--;
    event->time = timegm(&event->datetime);
    char *leftbracket = strchr(s, ']');
    event_str = leftbracket + 2;
    if (!strcmp(event_str, "falls asleep")) {
        event->type = EVENT_FALL_ASLEEP;
    } else if (!strcmp(event_str, "wakes up")) {
        event->type = EVENT_WAKEUP;
    } else {
        event->type = EVENT_BEGIN_SHIFT;
        sscanf(event_str, "Guard #%u begins shift", &event->guard_id);
        if (event->guard_id > highest_guard_id) {
            highest_guard_id = event->guard_id;
        }
    }
    
    line->extra = event;
    return true;
}

void free_event(line_t *line)
{
    free(line->extra);
}

char *event_make_string(event_t *event)
{
    static char buf[1024] = {0};
    switch (event->type) {
        case EVENT_WAKEUP:
            return "wakes up";
        case EVENT_FALL_ASLEEP:
            return "falls asleep";
        case EVENT_BEGIN_SHIFT:
            memset(buf, 0, sizeof(buf));
            snprintf(buf, 1024, "Guard #%u begins shift", event->guard_id);
            return buf;
        default:
            return "Unknown event?!?!";
    }
}

int sort_event(const void *a, const void *b)
{
    line_t *l1 = *(line_t **)a;
    line_t *l2 = *(line_t **)b;
    event_t *e1 = l1->extra;
    event_t *e2 = l2->extra;
    time_t t1 = e1->time;
    time_t t2 = e2->time;

    if (t1 == t2) {
        return 0;
    } else if (t1 < t2) {
        return -1;
    } else {
        return 1;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: %s [input]\n", getprogname());
        return EXIT_FAILURE;
    }

    file_t *file = file_get_lines(argv[1], parse_event, free_event, sort_event);
    DIE_IF((file == NULL), "Could not read lines from %s\n", argv[1]);

    line_t *line = NULL;
    size_t i = 0;
    file_sort_lines(file);
    guard_stats_t *stats = calloc(highest_guard_id, sizeof(guard_stats_t));

    uint32_t current_id = 0;
    uint32_t fall_asleep = 0;
    uint32_t wakeup = 0;
    guard_stats_t *most_sleepy_guard = NULL;
    minute_stats_t minutes[60];
    memset((void *)&minutes[0], 0, sizeof(minutes));
    uint32_t most_seen_minute = -1;
    uint32_t times_most_seen_minute = 0;
    file_for_each_line(file, line, i) {
        event_t *event = line_extra_data(line);
        switch (event->type) {
            case EVENT_BEGIN_SHIFT:
            {
                current_id = event->guard_id - 1;
                stats[current_id].guard_id = event->guard_id;
                if (most_sleepy_guard == NULL) {
                    most_sleepy_guard = &stats[current_id];
                }
                break;
            }
            case EVENT_FALL_ASLEEP:
            {
                fall_asleep = event->datetime.tm_min;
                break;
            }
            case EVENT_WAKEUP:
            {
                wakeup = event->datetime.tm_min;
                guard_stats_t *gs = &stats[current_id];
                gs->total_minutes_asleep += (wakeup - fall_asleep);
                if (gs->guard_id != most_sleepy_guard->guard_id) {
                    if (gs->total_minutes_asleep > most_sleepy_guard->total_minutes_asleep) {
                        most_sleepy_guard = gs;
                    }
                }
                for (uint32_t i = fall_asleep; i < wakeup; ++i) {
                    gs->minutes[i] += 1;
                    if (gs->minutes[i] > gs->minutes[gs->most_seen_minute]) {
                        gs->most_seen_minute = i;
                    }

                    if (minutes[i].times_asleep == 0) {
                        minutes[i].guard_id = gs->guard_id;
                        minutes[i].times_asleep = 1;
                    } else {
                        if (gs->minutes[i] > minutes[i].times_asleep) {
                            minutes[i].guard_id = gs->guard_id;
                            minutes[i].times_asleep = gs->minutes[i];
                        }
                    }

                    if (most_seen_minute == (uint32_t)-1) {
                        most_seen_minute = i; 
                        times_most_seen_minute = 1;
                    } else {
                        if (times_most_seen_minute < minutes[i].times_asleep) {
                            most_seen_minute = i;
                            times_most_seen_minute = minutes[i].times_asleep;
                        }
                    }
                }
                break;
            }
        }
    }

    printf("The most sleepy guard is %u with %u total minutes asleep, and is most often asleep at minute %u. Answer is %u\n", most_sleepy_guard->guard_id, most_sleepy_guard->total_minutes_asleep, most_sleepy_guard->most_seen_minute, (most_sleepy_guard->guard_id * most_sleepy_guard->most_seen_minute));
    printf("Guard %u spent minute %u more than any other guard or minute. Answer is %u\n", minutes[most_seen_minute].guard_id, most_seen_minute,(minutes[most_seen_minute].guard_id * most_seen_minute));
    free(stats);
    file_free(file);

    return 0;
}
