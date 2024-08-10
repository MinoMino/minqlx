import random
import string
from time import sleep
from unittest import TestCase

import redis

import database


def random_string():
    return "".join([random.choice(string.ascii_letters) for x in range(10)])


class RedisRegressionTestCase(TestCase):
    def setUp(self):
        self.c = database.Redis(None)
        self.c.connect("127.0.0.1:6379")

    def test_zadd_01(self):
        c = self.c
        key = "zadd_01_key_" + random_string()

        c.zadd(key, 5, "player1", 6, "player2")
        res = dict(c.zrange(key, 0, -1, withscores=True))
        self.assertEqual(res["player1"], 5)
        self.assertEqual(res["player2"], 6)

        c.zadd(key, 2, "player1")
        res = dict(c.zrange(key, 0, -1, withscores=True))
        self.assertEqual(res["player1"], 2)
        self.assertEqual(res["player2"], 6)

        c.zadd(key, 4, "player2")
        res = dict(c.zrange(key, 0, -1, withscores=True))
        self.assertEqual(res["player1"], 2)
        self.assertEqual(res["player2"], 4)

    def test_zadd_02(self):
        c = self.c
        key = "zadd_02_key_" + random_string()

        # redis-py 3.* introduced xx, nx, ch and incr kwarg params
        # this codes deliberately does not work in redis-py 2.*
        if redis.VERSION < (3,):
            self.skipTest("Old version of redis package: %s" % (redis.VERSION,))
        c.zadd(key, 5, "player1", 6, "player2")
        c.zadd(key, {"player1": 3}, xx=True)
        res = dict(c.zrange(key, 0, -1, withscores=True))
        self.assertEqual(res["player1"], 3)

        c.zadd(key, 3, "player_does_not_exist", xx=True)
        res = dict(c.zrange(key, 0, -1, withscores=True))
        self.assertNotIn("player_does_not_exist", res)

    def test_zincrby_01(self):
        key = "zincrby_01_key_" + random_string()
        c = self.c

        c.zincrby(key, "FIELD1")
        c.zincrby(key, "FIELD2", 2)
        res = dict(c.zrange(key, 0, -1, withscores=True))
        self.assertEqual(res["FIELD1"], 1)
        self.assertEqual(res["FIELD2"], 2)

        c.zincrby(key, "FIELD1")
        c.zincrby(key, "FIELD2", 3)
        res = dict(c.zrange(key, 0, -1, withscores=True))
        self.assertEqual(res["FIELD1"], 2)
        self.assertEqual(res["FIELD2"], 5)

    def test_lrem_01(self):
        key = "lrem_01_key_" + random_string()
        c = self.c

        c.rpush(
            key,
            "Cat",
            "Dog",
            "Horse",
            "Cat",
            "Dog",
            "Cat",
            "Monkey",
            "Dog",
            "Cat",
            "Dog",
            "Buffalo",
        )
        c.lrem(key, 2, "Cat")
        result = c.lrange(key, 0, -1)
        self.assertEqual(
            result,
            ["Dog", "Horse", "Dog", "Cat", "Monkey", "Dog", "Cat", "Dog", "Buffalo"],
        )

    def test_setex_01(self):
        key = "setex_01_key_" + random_string()
        c = self.c
        test_value = "test value"
        timeout = 1

        c.setex(key, timeout, test_value)
        self.assertEqual(c.get(key), test_value, "Too small timeout value?")
        sleep(timeout + 0.5)
        self.assertEqual(c.get(key), None)

    def test_mset_01(self):
        c = self.c
        c.mset(field1="example1", field2="example2")
        self.assertEqual(["example1", "example2"], c.mget("field1", "field2"))

    def test_mset_02(self):
        c = self.c
        c.mset({"field3": "example1", "field4": "example2"})
        self.assertEqual(["example1", "example2"], c.mget("field3", "field4"))

    def test_msetnx_01_kwargs(self):
        c = self.c
        prefix = "msetnx_01_key_" + random_string()
        key1 = prefix + "_field1"
        key2 = prefix + "_field2"
        d = {
            key1: "example1",
            key2: "example2",
        }
        c.msetnx(**d)
        self.assertEqual(["example1", "example2"], c.mget(key1, key2))

        d = {
            key1: "value should not be used",
            key2: "same here",
        }
        c.msetnx(**d)
        self.assertEqual(["example1", "example2"], c.mget(key1, key2))

    def test_msetnx_02_dict(self):
        c = self.c
        prefix = "msetnx_02_key_" + random_string()
        key1 = prefix + "_field1"
        key2 = prefix + "_field2"
        d = {
            key1: "example1",
            key2: "example2",
        }
        c.msetnx(d)
        self.assertEqual(["example1", "example2"], c.mget(key1, key2))

        d = {
            key1: "value should not be used",
            key2: "same here",
        }
        c.msetnx(d)
        self.assertEqual(["example1", "example2"], c.mget(key1, key2))
