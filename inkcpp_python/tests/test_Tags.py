import inkcpp_py as ink

class TestTags:
    @staticmethod
    def compare(obj):
        if isinstance(obj, ink.Runner):
            tags = obj.all_tags()
            assert obj.global_tags() == tags["global"]
            assert obj.num_global_tags() == len(tags["global"])
            assert obj.has_global_tags() == len(tags["global"]) > 0
            for (i, tag) in enumerate(tags["global"]):
                assert obj.get_global_tag(i) == tag
            assert obj.knot_tags() == tags["knot"]
            assert obj.num_knot_tags() == len(tags["knot"])
            assert obj.has_global_tags() == len(tags["knot"]) > 0
            for (i, tag) in enumerate(tags["knot"]):
                assert obj.get_knot_tag(i) == tag
            assert obj.tags() == tags["line"]
            assert obj.num_tags() == len(tags["line"])
            assert obj.has_tags() == len(tags["line"]) > 0
            for (i, tag) in enumerate(tags["line"]):
                assert obj.get_tag(i) == tag
        elif isinstance(obj, ink.Choice):
            tags = obj.tags()
            assert obj.has_tags() == len(tags) > 0
            assert obj.num_tags() == len(tags)
            for (i, tag) in enumerate(tags):
                assert obj.get_tag(i) == tag
        else:
            assert False
        
    def test_tags(self, assets, generate):
        [story, globals, runner] = generate(assets['TagsStory'])

        assert runner.all_tags() == {}
        self.compare(runner)

        expected = [("", {}), ("", {}), ("", {}), ("", {}), ("", {}), ("", {}), ("", {}), [("", []), ("", []), 1]]

        for state  in expected:
            if isinstance(state, tuple):
                (line, tags) = state
                assert runner.getline() == line
                assert runner.all_tags() == tags
                self.compare(runner)
            else:
                choose = state[-1]
                state = state[:-1]
                assert not runner.can_continue()
                assert runner.num_choices() == len(state)
                for ((text, tags), choice) in zip(state, iter(runner)):
                    assert choice.text() == text
                    assert choice.tags() == tags
                    self.compare(choice)
                runner.choose(choose)
                assert runner.can_continue()
                    

        
