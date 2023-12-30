import inkcpp_py as ink
import pytest
import os

@pytest.fixture
def inklecate_cmd():
    res = os.getenv("INKLECATE")
    if res is None or res == "":
        return "inklecate"
    return res

@pytest.fixture
def story_path(tmpdir):
    return list(map(lambda x: tmpdir / ("LookaheadSafe" + x), [".bin", ".tmp"])) + ["inkcpp_test/ink/LookaheadSafe.ink"]

@pytest.fixture
def assets(story_path, inklecate_cmd):
    if not os.path.exists(story_path[0]):
        if not os.path.exists(story_path[1]):
            os.system('{} -o {} {}'.format(inklecate_cmd, story_path[1], story_path[2]))
    ink.compile_json(str(story_path[1]), str(story_path[0]))
    ink_story = ink.Story.from_file(str(story_path[0]))
    ink_globals = ink_story.new_globals();
    ink_runner = ink_story.new_runner(ink_globals)
    return [ink_story, ink_globals, ink_runner]

class Cnt:
    def __init__(self):
        self.cnt = 0
    def __call__(self, _):
        self.cnt += 1
class TestExternalFunctions:
    def test_lookaheadSafe(self, assets):
        cnt = Cnt()
        assert len(assets) == 3
        [story, globals, runner] = assets
        runner.bind_void("foo", cnt, True)
        out = runner.getline()
        assert out == "Call1 glued to Call 2\n"
        assert cnt.cnt == 3
        out = runner.getline()
        assert out == "Call 3 is seperated\n"
        assert cnt.cnt == 4

    def test_lookahadeUnsafe(self, assets):
        cnt = Cnt()
        [story, globals, runner] = assets
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
    
