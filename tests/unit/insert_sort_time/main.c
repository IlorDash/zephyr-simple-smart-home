#include <limits.h>

#include <zephyr/ztest.h>

#include "app/src/read_queue/read_queue.h"

static const sensor_sampling time[] ={
	{1, 300, 900},{0, 1000, 1000},{2, 2000,2000},
}
static const sensor_sampling sampling_arr_ans1[] ={
	{1, 300, 900},{0, 1000, 1000},{2, 2000,2000},
}

static const sensor_sampling sampling_arr_test2[] ={
	{1, 300, 1200},{0, 1000, 1000},{2, 2000,2000},
}
static const sensor_sampling sampling_arr_ans2[] ={
	{0, 1000, 1000},{1, 300, 1200},{2, 2000,2000},
}

static const sensor_sampling sampling_arr_test3[] ={
	{0, 1000, 2000},{1, 300, 1200},{2, 2000,2000},
}
static const sensor_sampling sampling_arr_ans3[] ={
	{1, 300, 1200},{0, 1000, 2000},{2, 2000,2000},
}

static const sensor_sampling sampling_arr_test4[] ={
	{1, 300, 2100},{0, 1000, 2000},{2, 2000,2000},
}
static const sensor_sampling sampling_arr_ans4[] ={
	{0, 1000, 2000},{2, 2000,2000},{1, 300, 2100},
}

ZTEST(custom_lib, test_sort_samplings)
{
	/* Verify standard behavior */
	zassert_equal(sort_samplings_by_time(INT_MIN), INT_MIN, "get_value failed input of INT_MIN");

	zassert_equal(
		custom_get_value(INT_MIN + 1), INT_MIN + 1, "get_value failed input of INT_MIN + 1");

	zassert_equal(custom_get_value(-1), -1, "get_value failed input of -1");
	zassert_equal(custom_get_value(1), 1, "get_value failed input of 1");
	zassert_equal(
		custom_get_value(INT_MAX - 1), INT_MAX - 1, "get_value failed input of INT_MAX - 1");
	zassert_equal(custom_get_value(INT_MAX), INT_MAX, "get_value failed input of INT_MAX");
}

ZTEST_SUITE(custom_lib, NULL, NULL, NULL, NULL, NULL);
