import inkcpp_py as ink

class TestLists:
    def test_lists(self, assets, generate):
        [story, globals, runner] = generate(assets['ListStory'])

        val = globals.list
        l = val.as_list()
        hits = [False, False, False]
        for flag in l:
            if flag.name == 'bird' and flag.list_name == 'animals':
                hits[0] = True
            elif flag.name == 'red' and flag.list_name == 'colors':
                hits[1] = True
            elif flag.name == 'yellow' and flag.list_name == 'colors':
                hits[2] = True
            else:
                assert False
        assert hits[0] and hits[1] and hits[2]

        hits = [False, False]
        for flag in l.flags_from('colors'):
            if flag.name == 'red' and flag.list_name == 'colors':
                hits[0] = True
            elif flag.name == 'yellow' and flag.list_name == 'colors':
                hits[1] = True
            else:
                assert False
        assert hits[0] and hits[1]

        assert l.contains('yellow')
        assert not l.contains('white')

        l.add('white')
        l.remove('yellow')

        assert not l.contains('yellow')
        assert l.contains('white')

        globals.list = ink.Value(l)

        assert runner.getline() == 'cat, snake\n'
        assert runner.get_choice(0).text() == 'list: bird, white, red'
