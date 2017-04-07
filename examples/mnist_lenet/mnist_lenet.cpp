
#include "core/deep_flow.h"
#include <chrono>

#include <gflags/gflags.h>
#include <glog/logging.h>

DEFINE_bool(evaluate, false, "Evaluation only");
DEFINE_string(mnist_folder, "./data/mnist", "Path to Trained network model to load");
DEFINE_string(input_network_bin, "", "Trained network model to load");
DEFINE_string(output_network_bin, "", "Trained network model to save");
DEFINE_int32(batch, 100, "Batch size");

void main(int argc, char** argv) {
	gflags::ParseCommandLineFlags(&argc, &argv, true);

	//CudaHelper::setOptimalThreadsPerBlock();	

	int batch_size = FLAGS_batch;
	
	DeepFlow df;
	auto mnist_trainset = df.mnist_reader(FLAGS_mnist_folder, batch_size, MNISTReaderType::Train, "trainset", {"train"});
	auto mnist_testset = df.mnist_reader(FLAGS_mnist_folder, batch_size, MNISTReaderType::Test, "testset", {"test"});
	
	NodeOutputPtr x, y, loss, mse, accuracy;
	if (FLAGS_input_network_bin.empty()) {
		df.define_phase("test");
		df.define_phase("train");
		x = df.place_holder({ batch_size, 1, 28, 28 }, Tensor::TensorType::Float, "x");
		y = df.place_holder({ batch_size, 10, 1, 1 }, Tensor::TensorType::Float, "y");
		auto f1 = df.variable(df.random_uniform({ 20, 1 , 5, 5 }, -0.1f, 0.1f), "f1");
		auto conv1 = df.conv2d(x, f1, 2, 2, 1, 1, 1, 1, "conv1");
		auto pool1 = df.pooling(conv1, 2, 2, 0, 0, 2, 2, "pool1");
		auto f2 = df.variable(df.random_uniform({ 50, 20 , 5, 5 }, -0.1f, 0.1f), "f2");
		auto conv2 = df.conv2d(pool1, f2, "conv2");
		auto pool2 = df.pooling(conv2, 2, 2, 0, 0, 2, 2, "pool2");
		auto w1 = df.variable(df.random_uniform({ 1250, 500, 1 , 1 }, -0.1f, 0.1f), "w1");
		auto b1 = df.variable(df.step({ 1, 500, 1 , 1 }, -1.0f, 1.0f), "b1");
		auto relu1 = df.relu(df.bias_add(df.matmul(pool2, w1), b1), -0.01f, "relu1");
		auto dropout = df.dropout(relu1, 0.5f, "dropout");
		auto w2 = df.variable(df.random_uniform({ 500, 10, 1 , 1 }, -0.1f, 0.1f), "w2");
		auto b2 = df.variable(df.step({ 1, 10, 1, 1 }, -1.0f, 1.0f), "b2");
		auto relu2 = df.relu(df.bias_add(df.matmul(dropout, w2), b2), -0.01f, "relu2");
		loss = df.softmax_loss(relu2, y, "loss");		
		mse = df.reduce_mean(df.reduce_mean(df.square(loss), 1), 0, "mse", {"train"});
		auto correct_class = df.argmax(y, 1, "correct_class", {"train"});
		auto predict_class = df.argmax(loss->node()->output(1), 1, "predict_class");
		accuracy = df.reduce_mean(df.equal(predict_class, correct_class), 0, "accuracy", {"train"});
	}		

	df.global_node_initializer();

	auto trainer = df.gain_solver(loss, 2000, 0.9999f, 0.0001f, 100, 0.1f, 0.05f, 0.95f);
	//auto trainer = df.sgd_solver(loss, 2000, 0.9999f, 0.0001f);

	int iteration = 0;
	for (int epoch = 1; epoch <= 5; ++epoch) {
		bool exit = false;
		auto start = std::chrono::high_resolution_clock::now();
		while (!exit) {
			x->feed(mnist_trainset->output(0));
			y->feed(mnist_trainset->output(1));
			trainer->train_step();
			iteration++;
			if (mnist_trainset->isLastBatch())
				exit = true;
			mnist_trainset->nextBatch();
		}
		auto finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = finish - start;
		std::cout << "Epoch " << epoch << " ( Iteration: " << iteration << " )" << " Elapsed time: " << elapsed.count() << " seconds" << std::endl;
		//auto train_result = df.run(mse, accuracy, { { x,mnist_trainset->output(0) },{ y,mnist_trainset->output(1) } });
		//std::cout << "  Total Train Batches: " << std::get<2>(train_result) << " Train  MSE: " << std::get<0>(train_result) << " Train Accuracy: " << std::get<1>(train_result) * 100 << "%" << std::endl;
		auto test_result = df.run(mse, accuracy, { {x,mnist_testset->output(0)},{y,mnist_testset->output(1) } });						
		std::cout << "  Total  Test Batches: " << std::get<2>(test_result)  << "  Test  MSE: " << std::get<0>(test_result)  << "  Test Accuracy: " << std::get<1>(test_result)  * 100 << "%" << std::endl;
	}

	if (!FLAGS_output_network_bin.empty())
		df.save_as_binary(FLAGS_output_network_bin);
}

