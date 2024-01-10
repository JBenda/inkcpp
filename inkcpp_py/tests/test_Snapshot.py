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
        [story, _, runner] = generate(assets['SimpleStoryFlow'])

        runner.getline()
        assert runner.num_choices() == 3
        runner.choose(2)

        snap1 = runner.create_snapshot()

        cnt = 0
        while runner.can_continue():
            runner.getline()
            cnt += 1

        snap2 = runner.create_snapshot()

        check_end(runner)

        runner = story.new_runner_from_snapshot(snap1)
        while runner.can_continue():
            runner.getline()
            cnt -= 1
        assert cnt == 0

        check_end(runner)

        runner = story.new_runner_from_snapshot(snap2)
        assert not runner.can_continue()
        check_end(runner)
