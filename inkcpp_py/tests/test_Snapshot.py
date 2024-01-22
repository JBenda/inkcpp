import inkcpp_py as ink
import pytest

def check_end(runner):
    assert runner.num_choices() == 3
    runner.choose(2)
    while runner.can_continue():
        runner.getline()
    assert runner.num_choices() == 2

class TestSnapshot:
    def test_snapshot(self, assets, generate):
        [story, glob, runner] = generate(assets['SimpleStoryFlow'])
        runner2 = story.new_runner(glob)
        runner.getline()
        assert runner.num_choices() == 3
        snap0 = runner.create_snapshot()

        runner.choose(2)

        snap1 = runner.create_snapshot()

        cnt = 0
        while runner.can_continue():
            runner.getline()
            cnt += 1

        snap2 = runner.create_snapshot()

        check_end(runner)

        glob = story.new_globals_from_snapshot(snap0)
        runner = story.new_runner_from_snapshot(snap0, glob, 0)
        assert runner.num_choices() == 3
        runner.choose(2)
        cnt_x = 0
        while runner.can_continue():
            runner.getline()
            cnt_x += 1
        assert cnt_x == cnt

        check_end(runner)

        glob = story.new_globals_from_snapshot(snap1)
        runner = story.new_runner_from_snapshot(snap1, glob, 0)
        cnt_x = 0
        while runner.can_continue():
            runner.getline()
            cnt_x += 1
        assert cnt_x == cnt

        check_end(runner)

        glob = story.new_globals_from_snapshot(snap2)
        runner = story.new_runner_from_snapshot(snap2, glob, 0)
        assert not runner.can_continue()
        check_end(runner)
