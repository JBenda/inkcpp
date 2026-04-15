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
        [story, glob, runner] = generate(assets["SimpleStoryFlow"])
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

    def test_migration(self, assets, generate):
        [before_story, befero_glob, before_runner] = generate(assets["MigrationBefore"])
        after_story = assets["MigrationAfter"]
        assert before_runner.getall() == "We're going to the seaside!\n"
        assert before_runner.num_choices() == 3
        assert before_runner.get_choice(0).text() == "Make a sand castle"
        assert before_runner.get_choice(1).text() == "Go swimming"
        assert before_runner.get_choice(2).text() == "Time to go home"
        before_runner.choose(0)
        assert (
            before_runner.getall()
            == "We made a great sand castle, it even has a moat!\n"
            "We're going to the seaside!\nSo far we've done the following: SandCastle\n"
        )
        assert before_runner.num_choices() == 3
        assert before_runner.get_choice(0).text() == "Make a sand castle"
        assert before_runner.get_choice(1).text() == "Go swimming"
        assert before_runner.get_choice(2).text() == "Time to go home"
        before_runner.choose(1)
        snap = before_runner.create_snapshot()
        assert snap.can_be_migrated()
        assert (
            before_runner.getall() == "We swim and swam, it was delightful!\n"
            "We're going to the seaside!\nSo far we've done the following: Swimming, SandCastle\n"
        )
        assert before_runner.num_choices() == 2
        assert before_runner.get_choice(0).text() == "Make a sand castle"
        assert before_runner.get_choice(1).text() == "Time to go home"

        after_runner = after_story.new_runner_from_snapshot(snap)
        assert (
            after_runner.getall() == "We swim and swam, it was delightful!\n"
            "We're going to the seaside!\nSo far we've done the following: Swimming, SandCastle\n"
        )
        assert after_runner.num_choices() == 3
        assert after_runner.get_choice(0).text() == "Make a sand castle"
        assert after_runner.get_choice(1).text() == "Get Ice Cream"
        assert after_runner.get_choice(2).text() == "Time to go home"
        after_runner.choose(1)
        assert (
            after_runner.getall() == "We got ice cream, mine was raspberry!\n"
            "We're going to the seaside!\nSo far we've done the following: Swimming, SandCastle, IceCream\n"
        )
