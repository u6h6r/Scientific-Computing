    // void push(T const& data)
    // {
    //     std::unique_lock<std::mutex> lock_full(m_full);
    //     while(is_full())
    //     {   
    //         std::cout << "Waiting becasue queue is full\n";
    //         cv_full.wait(lock_full);
    //     }
    //     std::unique_lock<std::mutex> lock(m);
    //     std::queue< T >::push(data);
    //     bool const empty=std::queue< T >::empty();
    //     ++current;
    //     std::cout << "Array added to queue\n";
    //     lock.unlock(); // unlock the mutex
    //     if(!empty)
    //     {   
    //         std::cout << "Queue is not empty\n";
    //         cv.notify_all();
    //     }
    //     // if(is_full())
    //     // {
    //     // std::cout << "Queue is full\n";
    //     // }
    //     // std::this_thread::sleep_for(std::chrono::milliseconds(1000));