/**
 YaneuraOu���玝���Ă����x���`�R�}���h
*/

#include <vector>

#include "node.h"
#include "search.h"
#include "synced_printf.h"
#include "thinking.h"
#include "usi_protocol.h"
#include "misc.h"
// ----------------------------------
//  USI�g���R�}���h "bench"(�x���`�}�[�N)
// ----------------------------------

// benchmark�p�f�t�H���g�̋ǖʏW
// ����𑝂₷�Ȃ�A���̂ق��� fens.assign �̂Ƃ���̋ǖʐ������₷���ƁB
static const char* BenchSfen[] = {

	// �ǂ߂Γǂނقǌ�舫���悤�ȋǖ�
	"l4S2l/4g1gs1/5p1p1/pr2N1pkp/4Gn3/PP3PPPP/2GPP4/1K7/L3r+s2L w BS2N5Pb 1",

	// 57����͋l�݁A�݂����ȁB
	// �ǂ߂Γǂނقǐ�肪�������Ƃ��킩���Ă���ǖʁB
	"6n1l/2+S1k4/2lp4p/1np1B2b1/3PP4/1N1S3rP/1P2+pPP+p1/1p1G5/3KG2r1 b GSN2L4Pgs2p 1",

	// �w���萶���Ղ�̋ǖ�
	// cf. http://d.hatena.ne.jp/ak11/20110508/p1
	"l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w RGgsn5p 1",
};

void bench_cmd(Node* const node, UsiOptions* const usi_options, Thinking* const thinking)
{

	(*usi_options)["USI_Hash"] = std::string("256");
	(*usi_options)["Threads"] = std::string("1");

	thinking->Initialize();
	Evaluation::ReadParametersFromFile("params.bin");

	std::vector<std::string> fens(BenchSfen, BenchSfen + 3);

	UsiGoOptions go_options;
	go_options.depth = 15;
	int64_t nodes = 0;
	Timer time;

	time.reset();

	for (size_t i = 0; i < fens.size(); ++i)
	{
		*node = Node(Position::FromSfen(fens[i]));
		SYNCED_PRINTF("\nPosition: %d/%d\n", (i + 1), fens.size());

		thinking->StartThinking(*node, go_options);
		
		/*
		// �T������nps���\������邪�A����͂���global��Timer�Ɋ�Â��̂ŒT�����ƂɃ��Z�b�g���s�Ȃ��悤�ɂ���B
		Time.reset();

		Threads.start_thinking(pos, limits, st);
		Threads.main()->wait_for_search_finished(); // �T���̏I����҂B

		nodes += Threads.main()->rootPos.nodes_searched();
		*/
		// �m�[�h���̎������悤�킩���̂�NPS�͂Ȃ�
	}

	auto elapsed = time.elapsed() + 1; // 0���Z�̉���̂���

	// �o�ߎ��Ԃ̂ݕ\��
	SYNCED_PRINTF("time = %dms\n", elapsed);
}
