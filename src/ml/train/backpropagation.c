#include "backpropagation.h"

#define LEARNINGRATE 0.15

void trainingNetwork(struct Network *network, char *databasepath, size_t minibatchsize, size_t minibatchnumber, size_t minibatchtrain)
{
    //Define letters
    char *letters = malloc(62 * sizeof(char));
    size_t j = 0;
    for (char i = '0'; i <= '9'; i++) //Integers 0 to 9
    {
        letters[j] = i;
        j++;
    }
    for (char i = 'A'; i <= 'Z'; i++) //Uppercase letters
    {
        letters[j] = i;
        j++;
    }
    for (char i = 'a'; i <= 'z'; i++) //Lowercase letters
    {
        letters[j] = i;
        j++;
    }
    //Fresh neurones
    for (size_t i = 1; i < network->nb_layers; i++)
    {
        for (size_t n = 0; n < network->layers[i].nb_neurones; n++)
        {
            if ((network->layers[i].neurones[n].delta_bias) != NULL && network->layers[i].neurones[n].delta_weight != NULL)
            {
                free(network->layers[i].neurones[n].delta_bias);
                free(network->layers[i].neurones[n].delta_weight);
            }
            network->layers[i].neurones[n].delta_bias = malloc(minibatchsize * sizeof(double));
            network->layers[i].neurones[n].delta_weight = malloc(network->layers[i - 1].nb_neurones * minibatchsize * sizeof(double));
        }
    }

    for (size_t nb = 0; nb < minibatchnumber; nb++)
    {
        char *desired_output = malloc(sizeof(char) * minibatchsize);
        double **inputs = malloc(sizeof(double *) * minibatchsize);
        for (size_t i = 0; i < minibatchsize; i++)
        {
            //Define minibatch
            desired_output[i] = rand() % 62;
            inputs[i] = loadDataBase(databasepath, letters[(size_t)desired_output[i]], rand() % 1000);
        }

        printf("minibatch num°%lu\n", nb);
        for (size_t j = 0; j < minibatchtrain; j++)
        {
            minibatch(network, minibatchsize, letters, desired_output, inputs);
        }
        free(desired_output);
        for (size_t i = 0; i < minibatchsize; i++)
        {
            free(inputs[i]);
        }
        free(inputs);
        char *path = malloc(sizeof(char)*100);
        sprintf(path,"network.minibatch%lu.json",nb);
        SaveNetworkToJson(network, path);
    }
    free(letters);
}

void minibatch(struct Network *network, size_t minibatchsize, char *letters, char *desired_output, double **inputs)
{

    for (size_t i = 0; i < minibatchsize; i++)
    {
        //Define minibatch TO UP
        char letter = letters[(size_t)desired_output[i]];
        double *input = inputs[i];

        //Feedforward (run the network with input to set the z and activation values)
        double *output = calculateNetworkOutput(network, input);

        printf("Lettre %c:\n", letter);
        //PrintInput(input, 128, 128);
        PrintOuput(output, letters, 62);


        //Output error (calculation delta of the last layer) delta = (activation - outputTarget) * actvation_fonction'(z)
        for (size_t k = 0; k < networkNbOutput(network); k++)
        {
            struct Neurone *n = &(network->layers[network->nb_layers - 1].neurones[k]);
            n->delta_error = (output[k] - (k == (size_t)desired_output[i]));

            n->delta_error *= actvation_fonction_derivate(n);

            //printf("%f %f; ", n->bias, n->delta_error);
        }

        free(output);

        //Backpropagate the error
        backpropagation(network);

        //Set delta
        for (size_t l = 1; l < network->nb_layers; l++)
        {
            for (size_t n = 0; n < network->layers[l].nb_neurones; n++)
            {
                struct Neurone *neurone = &(network->layers[l].neurones[n]);
                neurone->delta_bias[i] = neurone->delta_error;

                for (size_t k = 0; k < network->layers[l - 1].nb_neurones; k++)
                {
                    neurone->delta_weight[k * minibatchsize + i] = activationFunction(&(network->layers[l - 1].neurones[k])) * neurone->delta_error;
                }
            }
        }

        //PrintNetwork(network);
    }

    //Average bias and weight
    for (size_t l = 1; l < network->nb_layers; l++)
    {
        for (size_t n = 0; n < network->layers[l].nb_neurones; n++)
        {
            struct Neurone *neurone = &(network->layers[l].neurones[n]);
            double sumbias = 0;

            for (size_t k = 0; k < network->layers[l - 1].nb_neurones; k++)
            {
                double sumweights = 0;

                for (size_t i = 0; i < minibatchsize; i++)
                {

                    sumweights += neurone->delta_weight[k * minibatchsize + i];
                }

                neurone->weights[k] -= LEARNINGRATE * (sumweights / minibatchsize);
            }
            for (size_t i = 0; i < minibatchsize; i++)
            {
                sumbias += neurone->delta_bias[i];
            }
            neurone->bias -= LEARNINGRATE * (sumbias / minibatchsize);
        }
    }
}

void backpropagation(struct Network *network)
{
    //For each l=L−1,L−2,…,2 compute deltal = (45)
    //struct Layer* layer = network.layers;
    for (size_t l = network->nb_layers - 2; l > 0; l--)
    {
        for (size_t j = 0; j < network->layers[l].nb_neurones; j++)
        {
            double sum = 0;
            for (size_t k = 0; k < network->layers[l + 1].nb_neurones; k++)
            {
                sum += network->layers[l + 1].neurones[k].weights[j] * network->layers[l + 1].neurones[k].delta_error;
            }

            network->layers[l].neurones[j].delta_error = sum * actvation_fonction_derivate(&network->layers[l].neurones[j]);
        }
    }
}

double *loadDataBase(char *databasepath, char letter, size_t imagenumber)
{
    //Convert a imagenumber to a "12345" string
    char *imagename = malloc(6 * sizeof(char));
    imagename[5] = 0;
    for (size_t i = 5; i > 0; i--)
    {
        imagename[i - 1] = '0' + imagenumber % 10;
        imagenumber /= 10;
    }

    //Build the image path string
    char *imagepath = malloc(150 * sizeof(char));
    sprintf(imagepath, "%s/%02x/train_%02x/train_%02x_%s.png", databasepath, letter, letter, letter, imagename);

    //Load image
    SDL_Surface *image = loadImage(imagepath);
    free(imagepath);
    free(imagename);
    double *imagebin = binarizationpointer(image);
    SDL_FreeSurface(image);
    return imagebin;
}

void PrintInput(double *input, size_t height, size_t with)
{
    printf("Input:\n");
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < with; j++)
        {
            if (*(input + i * with + j) > 0.5)
                printf("%i", 1);
            else
                printf(" ");
        }
        printf("|\n");
    }
    scanf("\n");
}

void PrintOuput(double *output, char *letters, size_t size)
{
    printf("Output: { ");
    for (size_t i = 0; i < size; i++)
    {
        printf("%c: %f; ", *(letters + i), *(output + i));
    }
    printf("}\n");
}