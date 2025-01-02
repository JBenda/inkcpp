import inkcpp_py as ink
import pytest
import os

class Cnt:
    def __init__(self):
        self.cnt = 0
    def __call__(self, _):
        self.cnt += 1
class TestExternalFunctions:
    def test_lookaheadSafe(self, assets, generate):
        cnt = Cnt()
        [story, globals, runner] = generate(assets['LookaheadSafe'])
        runner.bind_void("foo", cnt, True)
        out = runner.getline()
        assert out == "Call1 glued to Call 2\n"
        assert cnt.cnt == 3
        out = runner.getline()
        assert out == "Call 3 is seperated\n"
        assert cnt.cnt == 4

    def test_lookahadeUnsafe(self, assets, generate):
        cnt = Cnt()
        [story, globals, runner] = generate(assets['LookaheadSafe'])
        runner.bind_void("foo", cnt)
        out = runner.getline()
        assert out == "Call1\n"
        assert cnt.cnt == 1
        out = runner.getline()
        assert out == "glued to Call 2\n"
        assert cnt.cnt == 2
        out = runner.getline()
        assert out == "Call 3 is seperated\n"
        assert cnt.cnt == 3
    
